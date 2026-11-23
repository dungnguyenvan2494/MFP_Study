#this file is '.profile'
#to use rename root.profile to .profile

msg=`cat /proc/cmdline`
set -- $msg

echo "ulimit -n 8192"
ulimit -n 8192
echo "start haveged"
haveged_daemon

UART_COMMON_PARAM="19200 parenb -parodd cs8 -hupcl -cstopb cread clocal -crtscts -ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff -iuclc -ixany -imaxbel -iutf8 -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt -echoctl -echoke"

# D103の設定.
stty -F /dev/ttyD103 ${UART_COMMON_PARAM}

# EDH・地紋の設定.
stty -F /dev/ttyEDH  ${UART_COMMON_PARAM}
stty -F /dev/ttySEC1 ${UART_COMMON_PARAM}
stty -F /dev/ttySEC2 ${UART_COMMON_PARAM}

# デバッグターミナルがttyS5時はCSRCを無効にする.
cat /proc/consoles | grep ttyS5 > /dev/null
if [ 0 -eq $? ]; then
	# シンボリックリンクを消しておく.
	rm /dev/ttyCSRC
else
	stty -F /dev/ttyCSRC 38400 -parenb -parodd cs8 hupcl -cstopb cread clocal crtscts -ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr icrnl ixon -ixoff -iuclc -ixany -imaxbel -iutf8 opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 isig icanon iexten -echo echoe echok -echonl -noflsh -xcase -tostop -echoprt echoctl echoke
fi

# ブートパラメータからサブフォルダ名を取得.
for arg in $@
do
	case $arg in
	"FwDir="*)
		FwDir=${arg#*=};;
	esac
done

# 高速ダウンロードRom判定用フラグを初期化.
export IsExpRom=0

if [ -n "$SSH_TTY" ]; then
	screen -x
fi

export KMSWAP=0			# EagleH_制御構成変更_No.49 swap有効化対応(まずはswap機能を無効にしておく)

chmod 707 /dev/shm
chmod 707 /dev/input

if [ $1 = "boottyp=0" ]; then
	echo " 0 DEF_BOOT + Int"

	# EagleH_制御構成変更_No.49 swap有効化対応↓

	# SSD起動ならswap有効化する
	if [[ ! -e /sys/block/kmsda/device/type ]]; then
		# swap機能を有効にする
		KMSWAP=1

		# swap機能初期化(swapファイル作成、swapフォーマット、swappiness設定、swap有効化)
		syncswap.sh 2

		# swap有効時にCGROUPのメモリ機能も有効にする
		# CGROUPをマウントする
		mount -t cgroup -o memory cgroup /cgroup_mem			# マウント
		mkdir /cgroup_mem/swap						# swapグループを作成
		mkdir /cgroup_mem/noswap					# noswapグループを作成

		memsize=`free -b | grep Mem | awk '{ print $2 }'`		# フリーメモリサイズを取得

		# noswapグループを設定する
		echo $memsize > /cgroup_mem/noswap/memory.limit_in_bytes	# 物理をフリーメモリサイズ全て
		echo $memsize > /cgroup_mem/noswap/memory.memsw.limit_in_bytes	# 仮想をフリーメモリサイズ全て
		echo 1 > /cgroup_mem/noswap/memory.oom_control			# OOM Killerは無効
		echo 3 > /cgroup_mem/noswap/memory.move_charge_at_immigrate	# チャージは移行先に準ずる
		echo 0 > /cgroup_mem/noswap/memory.swappiness			# swapは出来るだけ抑止する

		# swapグループを設定する
		if [ $6 = "MachineInfo=12" ]; then
			#Deneb系
			echo 400m > /cgroup_mem/swap/memory.limit_in_bytes		# 物理を0.4GB
			echo 1900m > /cgroup_mem/swap/memory.memsw.limit_in_bytes	# 仮想を1.5GB
		elif  [ $6 = "MachineInfo=11" ] || [ $6 = "MachineInfo=14" ] ; then
			#Sparrow系
			echo 700m > /cgroup_mem/swap/memory.limit_in_bytes		# 物理を0.7GB
			echo 2200m > /cgroup_mem/swap/memory.memsw.limit_in_bytes	# 仮想を1.5GB
		elif  [ $6 = "MachineInfo=15" ]; then
			#Helios系
			echo 1200m > /cgroup_mem/swap/memory.limit_in_bytes		# 物理を1.2GB
			echo 2700m > /cgroup_mem/swap/memory.memsw.limit_in_bytes	# 仮想を1.5GB
		else
			#それ以外
			echo 1300m > /cgroup_mem/swap/memory.limit_in_bytes		# 物理を1.3GB
			echo 2800m > /cgroup_mem/swap/memory.memsw.limit_in_bytes	# 仮想を1.5GB
		fi

		echo 1 > /cgroup_mem/swap/memory.oom_control			# OOM Killerは無効
		echo 3 > /cgroup_mem/swap/memory.move_charge_at_immigrate	# チャージは移行先に準ずる
		echo 10 > /cgroup_mem/swap/memory.swappiness			# swapは出来るだけ抑止する

		# 全スレッドを noswap グループへ割り当てる。
		for I in `ps -eLo tid`; do echo $I > /cgroup_mem/noswap/tasks 2>/dev/null; done

		chmod -R uo=rwX,g= /cgroup_mem

		#Sanitizer有効時は、本体FW起動中にメモリ不足となる為、起動前にスワップを有効化する.
		if [ -f /km/fw/bin/SanitizerEnable ] ; then
			syncswap.sh 1
		fi
	fi
	# EagleH_制御構成変更_No.49 swap有効化対応↑

	exec /etc/mfpintegrity.sh 0 /km/ /km/fw/bin/mfp000_hwQt
else
if [ $1 = "boottyp=1" ]; then
	echo " 1 USB_BOOT"
	exec /etc/mfpAutoStart.sh /km/fw/bin/mfp000_hwQt
else
if [ $1 = "boottyp=2" ]; then
	echo " 2 USB_UPDATE [Tick:"`cat /sys/kernel/km_tick/tick_value`"]"

	while read line; do
		line=${line%?}
		case $line in
		"@EXP"*)
			IsExpRom=1
			;;
		"@MS1"*)
			FileName_MS1=${line#*FN=}
			;;
		"@SUB"*)
			FileName_SUB=${line#*FN=}
			;;
		esac
	done < /bd/sdc1/$FwDir/INDEX

	# 完全性チェック実行先・起動Subsetのパスにサブフォルダを設定.
	if [ $IsExpRom -eq 1 ]; then
		mount -t cramfs -o loop /bd/sdc1/$FwDir/$FileName_SUB /km

		if [ -z "$FileName_MS1" ]; then
			echo "Error. INDEX File invalid. Not found SubsetPath. "
		else
			exec /etc/mfpintegrity.sh 4 /bd/sdc1/$FwDir/ /km/$FileName_MS1
		fi
	else
		mount -t vfat /dev/sdc1 /km

		if [ -z "$FileName_MS1" ]; then
			echo "Error. INDEX File invalid. Not found SubsetPath. "
		else
			exec /etc/mfpintegrity.sh 4 /km/$FwDir/ /km/$FwDir/$FileName_MS1
		fi
	fi

else
if [ $1 = "boottyp=3" ]; then
	echo " 3 DEF_IISW"
	mount -t cramfs -o loop /bd/sda1/SUB.img /km
	exec /etc/mfpAutoStart.sh /km/fw/bin/mfp001_hwQt
else
if [ $1 = "boottyp=4" ]; then
	echo " 4 BOOT_LINE"
	KEY_INPUT_VALUE=`od /dev/kmsda -j33653 -N1 -tx1 -A n`
	UTILITY_KEY_FLAG=`cat /sys/bus/platform/devices/PMU.0/devices/UTILITY_KEY`
	if [ $KEY_INPUT_VALUE = "00" ]; then
		if [ $UTILITY_KEY_FLAG = "on" ]; then
			if [ -e /ata0a/SPI-FlashDataBackUpDir ]; then
				export LINE_BACKUP_RESTORE="1"
			fi
		fi
	fi
	screen /etc/mfpAutoStart.sh /km/fw/bin/mfp000_hwQt
#	exec /etc/mfpAutoStart.sh /km/fw/bin/mfp000_hwQt
else
if [ $1 = "boottyp=5" ]; then
	echo " 5 BOOT_ERR"
	mount -t cramfs -o loop /bd/sda1/SUB.img /km
	exec /etc/mfpAutoStart.sh /km/fw/bin/mfp001_hwQt
else
if [ $1 = "boottyp=7" ]; then
	echo " 7 BOOT_HW_CHECKMODE"
	exec /etc/mfpAutoStart.sh /km/fw/bin/mfp000_hwQt
else
if [ $1 = "boottyp=8" ]; then
	echo " 8 BOOT_SPEEDCHANGE"
	exec /etc/mfpAutoStart.sh /km/fw/bin/mfp000_hwQt
else
if [ $1 = "boottyp=9" ]; then
	echo " 9 DEF_UPDATE"

	while read line; do
		line=${line%?}
		case $line in
		"@EXP"*)
			IsExpRom=1
			;;
		"@MS1"*)
			FileName_MS1=${line#*FN=}
			;;
		"@SUB"*)
			FileName_SUB=${line#*FN=}
			;;
		esac
	done < /bd/sda1/INDEX

	if [ $IsExpRom -eq 1 ]; then
		mount -t cramfs -o loop /bd/sda1/$FileName_SUB /km
	fi

	if [ -z "$FileName_MS1" ]; then
		echo "Error. INDEX File invalid. Not found SubsetPath. "
	else
#		更新起動前のFW展開時に完全性チェックを実行している為、ここでは行わない.
#		exec /etc/mfpintegrity.sh 1 /km/$FileName_MS1
		exec /etc/mfpAutoStart.sh /km/$FileName_MS1
	fi
else
if [ $1 = "boottyp=11" ]; then
	echo " 11 DEF_BACKUP"
	mount -t cramfs -o loop /bd/sda1/SUB.img /km
	exec /etc/mfpAutoStart.sh /km/fw/bin/mfp001_hwQt
else
if [ $1 = "boottyp=12" ]; then
	echo " 12 DEF_RESTORE"
	mount -t cramfs -o loop /bd/sda1/SUB.img /km
	exec /etc/mfpAutoStart.sh /km/fw/bin/mfp001_hwQt
else
	echo " ERR: Boot Error."
	exec /etc/mfpAutoStart.sh /km/fw/bin/mfp000_hwQt
fi
fi
fi
fi
fi
fi
fi
fi
fi
fi
fi

