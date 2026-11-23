// SPDX-License-Identifier: GPL-2.0
/*
 * Marvell Quartz Clk gate driver
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/io.h>

/* AutoMode: SET when CLR CLKEN, CLR when SET CLKEN */
#define CLK_AUTOMODE_MASK 	(1 << 29)
#define CLK_EN_MASK 		(1 << 1)
#define CLK_RESET_MASK		(1 << 0)
#define CLK_GEN_DIV_MASK	(0xFFFF << 8)

#define CLK_STATUS_OFFSET		0x4
#define CLK_STATUS_SUCCESS_MASK		0x8
#define CLK_STATUS_FAIL_MASK		0x10

#define to_quartz_clkgate(_hw) container_of(_hw, struct quartz_clkgate, hw)
struct quartz_clkgate {
	struct clk_hw	hw;
	void __iomem	*config;
	int		reset;	       /* allow control of domain reset */
	int		ignore_enable; /* let LPP handle the ENABLE device */
	int		ignore_disable;/* let LPP handle the DISABLE device */
};

static bool debug_clkdisable;
module_param(debug_clkdisable, bool, 0644);

static int quartz_clkgate_is_enabled(struct clk_hw *hw)
{
	struct quartz_clkgate *gate = to_quartz_clkgate(hw);
	int reset;
	int val = readl(gate->config);

	/*
	 * If this is a clock with a reset, read the reset value
	 */
	if (gate->reset)
		reset = !!(val & CLK_RESET_MASK);
	else
		reset = 1;

	pr_err("%s: %s@%pK value %#08x\n", __func__, __clk_get_name(hw->clk),
	    gate->config, val);

	/*
	 * Read the enable bit
	 */
	return (val & CLK_EN_MASK) ? reset : 0;
}

static int quartz_clkgate_enable(struct clk_hw *hw)
{
	struct quartz_clkgate *gate = to_quartz_clkgate(hw);
	int val = readl(gate->config);

	if (gate->ignore_enable) {
		pr_err("%s: ignore %s@%pK value %#08x\n", __func__,
		    __clk_get_name(hw->clk),  gate->config, val);
		return 0;
	}

	/*
	 * Set the enable bit, clr Auto_mode
	 */
	val |= CLK_EN_MASK;
	val &= ~CLK_AUTOMODE_MASK;
	writel(val, gate->config);

	pr_err("%s: %s@%pK value %#08x\n", __func__, __clk_get_name(hw->clk),
	    gate->config, val);

	/*
	 * If this is a clock with a reset, set that too
	 */
	if (gate->reset) {
		val |= CLK_RESET_MASK;
		writel(val, gate->config);
	}

	return 0;
}

static void quartz_clkgate_disable(struct clk_hw *hw)
{
	struct quartz_clkgate *gate = to_quartz_clkgate(hw);
	int val = readl(gate->config);

	if (gate->ignore_disable) {
		pr_err("%s: ignore %s@%pK value %#08x\n", __func__,
		    __clk_get_name(hw->clk),  gate->config, val);
		return;
	}

	/*
	 * If this is a clock with a reset, clear it first
	 */
	if (gate->reset) {
		val &= ~CLK_RESET_MASK;
		writel(val, gate->config);
	}

	/*
	 * Clear the enable bit, always set automode
	 * this allow clock to auto disable w/o f/w handshake.
	 */
	val &= ~CLK_EN_MASK;
	val |= CLK_AUTOMODE_MASK;

	writel(val, gate->config);

	pr_err("%s: %s@%pK value %#08x\n", __func__, __clk_get_name(hw->clk),
	    gate->config, val);

	if (debug_clkdisable) {
		val = readl(gate->config + CLK_STATUS_OFFSET);
		WARN(!(val & CLK_STATUS_SUCCESS_MASK),
		     "quartz clock %s@%pK disable failed. Status = 0x%x",
		     __clk_get_name(hw->clk), gate->config, val);
	}
}

const struct clk_ops quartz_clkgate_ops = {
	.is_enabled = quartz_clkgate_is_enabled,
	.enable = quartz_clkgate_enable,
	.disable = quartz_clkgate_disable,
};

static void __init of_quartz_clkgate_setup(struct device_node *node)
{
	struct quartz_clkgate *gate;
	struct clk *clk;
	void __iomem *clk_base;
	/* struct clk *parent_clk; */
	struct clk_init_data *init;
	/* const char *parent_name; */
	int always_used;
	u32 init_value;

	pr_err("*******%s %d: Reached here\n", __func__, __LINE__);

	gate = kzalloc(sizeof(*gate), GFP_KERNEL);
	if (!gate)
		return;

	init = kzalloc(sizeof(*init), GFP_KERNEL);
	if (!init)
		goto free_out;

	/*
	 * If the reset property is set, this is as clock with a sw reset bit
	 */
	if (of_property_read_u32(node, "reset", &gate->reset))
		gate->reset = 0;

	always_used  = of_property_read_bool(node, "always-used");

	clk_base = of_iomap(node, 0);
	if (WARN_ON(!clk_base))
		goto free_out2;

	init->name = node->name;
	init->ops = &quartz_clkgate_ops;
	/* init->flags = CLK_SET_RATE_PARENT; */
	init->flags = CLK_IS_ROOT | CLK_IS_BASIC;
	/* parent_clk = of_clk_get(node, 0);
	 * parent_name = __clk_get_name(parent_clk);
	 * init->parent_names = &parent_name;
	 * init->num_parents = 1;
	 */
	init->parent_names = NULL;
	init->num_parents = 0;

	gate->hw.init = init;
	gate->config = clk_base;
	if (!of_property_read_u32(node, "initial-value",&init_value)) {
		// ONLY take the initial value divisor information...
		int val = readl(gate->config);
		val = (val & ~CLK_GEN_DIV_MASK) | (init_value & CLK_GEN_DIV_MASK);
		writel(val, gate->config);
	}
	if (!of_property_read_u32(node, "ignore-disable",&gate->ignore_disable)) {
		pr_err("%s: ignore_disable: %d, @%pK\n", __func__,
		    gate->ignore_disable, gate->config);
	}
	if (!of_property_read_u32(node, "ignore-enable",&gate->ignore_enable)) {
		pr_err("%s: ignore_enable:  %d, @%pK\n", __func__,
		    gate->ignore_enable, gate->config);
	}

	clk = clk_register(NULL, &gate->hw);
	if (WARN_ON(IS_ERR(clk)))
		goto map_out;

	if (always_used)
		clk_prepare_enable(clk);

	of_clk_add_provider(node, of_clk_src_simple_get, clk);

	return;
map_out:
	iounmap(clk_base);
free_out2:
	kfree(init);
free_out:
	kfree(gate);
}

CLK_OF_DECLARE(quartz_clkgate, "marvell,quartz-clkgate",
	       of_quartz_clkgate_setup);
