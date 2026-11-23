/*
 * Copyright (C) 2013-2015 Fujitsu Semiconductor Ltd.
 * Copyright (C) 2015 Linaro Ltd.
 * Author: Jassi Brar <jaswinder.singh@linaro.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/amba/bus.h>
#include <linux/mailbox_controller.h>

#ifndef MHU_CHANS
#error "compile arm_mhu.c/mv_mhu.c"
#endif

struct mhu_link {
	unsigned irq;
	void __iomem *tx_reg;
	void __iomem *rx_reg;
};

struct arm_mhu {
	void __iomem *base;
	struct mhu_link mlink[MHU_CHANS];
	struct mbox_chan chan[MHU_CHANS];
	struct mbox_controller mbox;
};

static irqreturn_t mhu_rx_interrupt(int irq, void *p)
{
	struct mbox_chan *chan = p;
	struct mhu_link *mlink = chan->con_priv;
	u32 val;

	val = readl_relaxed(mlink->rx_reg + INTR_STAT_OFS);
	if (!val)
		return IRQ_NONE;

	mbox_chan_received_data(chan, (void *)&val);

	writel_relaxed(val, mlink->rx_reg + INTR_CLR_OFS);
	return IRQ_HANDLED;
}

/* 2019/10/09 OP_BTS-15484 山口 CA72のウォッチドッグ範囲拡張 */
#define	WDT_CA72_TIME_Watch		(10)
#define WDT_CA72_TIME_WaitCntr	(100)

extern void PMU_Reboot(void);
extern void emergency_restart(void) ;

/* 本関数が呼び出される区間では割り込みが入らない */
/* 割り込みではなく関数コールでLPPPからの応答を受け取る */
static int mhu_wdt_ca72_end( struct mbox_chan *chan)
{
	struct mhu_link *mlink = chan->con_priv;
	u32 val;
	u32 time_cnt=0;
	
	while(1){
		val = readl_relaxed(mlink->rx_reg + INTR_STAT_OFS);
		if( val == 1){
			break;
		}
		if(time_cnt > WDT_CA72_TIME_WaitCntr ){
			emergency_restart();
			return -EIO;
		}
		time_cnt++;
		msleep(WDT_CA72_TIME_Watch);
	}
	mbox_chan_received_data(chan, (void *)&val);

	writel_relaxed(val, mlink->rx_reg + INTR_CLR_OFS);
	
	return 0;
	
}

static bool mhu_last_tx_done(struct mbox_chan *chan)
{
	struct mhu_link *mlink = chan->con_priv;
	u32 val = readl_relaxed(mlink->tx_reg + INTR_STAT_OFS);

	return (val == 0);
}

static int mhu_send_data(struct mbox_chan *chan, void *data)
{
	struct mhu_link *mlink = chan->con_priv;
	u32 *arg = data;

	writel_relaxed(*arg, mlink->tx_reg + INTR_SET_OFS);

	return 0;
}

static int mhu_startup(struct mbox_chan *chan)
{
	struct mhu_link *mlink = chan->con_priv;
	u32 val;
	int ret;

	val = readl_relaxed(mlink->tx_reg + INTR_STAT_OFS);
	writel_relaxed(val, mlink->tx_reg + INTR_CLR_OFS);

	if (mlink->irq == 0)
		return 0;  /* doing polling, no irq */

	ret = request_irq(mlink->irq, mhu_rx_interrupt,
			  IRQF_SHARED, "mhu_link", chan);
	if (ret) {
		dev_err(chan->mbox->dev,
			"Unable to acquire IRQ %d\n", mlink->irq);
		return ret;
	}
	return 0;
}

static void mhu_shutdown(struct mbox_chan *chan)
{
	struct mhu_link *mlink = chan->con_priv;

	free_irq(mlink->irq, chan);
}

static const struct mbox_chan_ops mhu_ops = {
	.send_data = mhu_send_data,
	.startup = mhu_startup,
	.shutdown = mhu_shutdown,
	.last_tx_done = mhu_last_tx_done,
	.wdt_ca72_end = mhu_wdt_ca72_end,		/* 2019/10/09 OP_BTS-15484 山口 CA72のウォッチドッグ範囲拡張 */
};

static int mhu_probe(struct amba_device *adev, const struct amba_id *id)
{
	int i, err;
	struct arm_mhu *mhu;
	struct device *dev = &adev->dev;
	int mhu_reg[] = {MHU_LP_OFFSET, MHU_HP_OFFSET, MHU_SEC_OFFSET};
	printk("[CT002][TC001] Enter mhu_probe\n");
	/* Allocate memory for device */
	mhu = devm_kzalloc(dev, sizeof(*mhu), GFP_KERNEL);
	if (!mhu)
		return -ENOMEM;

	mhu->base = devm_ioremap_resource(dev, &adev->res);
	if (IS_ERR(mhu->base)) {
		dev_err(dev, "ioremap failed\n");
		return PTR_ERR(mhu->base);
	}

	for (i = 0; i < MHU_CHANS; i++) {
		mhu->chan[i].con_priv = &mhu->mlink[i];
		mhu->mlink[i].irq = adev->irq[i];
		mhu->mlink[i].rx_reg = mhu->base + mhu_reg[i] + RX_REG_OFFSET;
		mhu->mlink[i].tx_reg = mhu->base + mhu_reg[i] + TX_REG_OFFSET;
	}

	mhu->mbox.dev = dev;
	mhu->mbox.chans = &mhu->chan[0];
	mhu->mbox.num_chans = MHU_CHANS;
	mhu->mbox.ops = &mhu_ops;
	mhu->mbox.txdone_irq = false;
	mhu->mbox.txdone_poll = true;
	mhu->mbox.txpoll_period = 1;

	amba_set_drvdata(adev, mhu);

	err = mbox_controller_register(&mhu->mbox);
	if (err) {
		dev_err(dev, "Failed to register mailboxes %d\n", err);
		return err;
	}
	printk("[CT002][TC001] Exit mhu_probe\n");
	return 0;
}

static void mhu_remove(struct amba_device *adev)
{
	struct arm_mhu *mhu = amba_get_drvdata(adev);

	mbox_controller_unregister(&mhu->mbox);
}

static struct amba_id mhu_ids[] = {
	{
		.id	= 0x1bb098,
		.mask	= 0xffffff,
	},
	{ 0, 0 },
};
MODULE_DEVICE_TABLE(amba, mhu_ids);

static struct amba_driver arm_mhu_driver = {
	.drv = {
		.name	= "mhu",
	},
	.id_table	= mhu_ids,
	.probe		= mhu_probe,
	.remove		= mhu_remove,
};
module_amba_driver(arm_mhu_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ARM MHU Driver");
MODULE_AUTHOR("Jassi Brar <jassisinghbrar@gmail.com>");
