#!/bin/bash

while [[ $# -gt 0 ]]
do
	key="$1"
	case "$key" in
		"-s"|"--sysroot-dir")
			SYSROOT_DIR="$2"
			shift 2
		;;
		"-b"|"--bin-name")
			BIN_NAME="$2"
			shift 2
		;;
		"-o"|"--out-iso-path")
			OUT_ISO_PATH="$2"
			shift 2
		;;
		"-B"|"--boot-spec")
			BOOT_SPEC="$2"
			shift 2
		;;
		*)
			shift
		;;
	esac
done

if [ -z "$SYSROOT_DIR" ]; then
	echo "[iso.sh] SYSROOT_DIR unset"
	exit 1
elif [ -z "$OUT_ISO_PATH" ]; then
	echo "[iso.sh] OUT_ISO_PATH unset"
	exit 2
elif [ -z "$BIN_NAME" ]; then
	echo "[iso.sh] BIN_NAME unset"
	exit 3
elif [ -z "$BOOT_SPEC" ]; then
	echo "[iso.sh] BOOT_SPEC unset"
	exit 4
fi

if [ "$BOOT_SPEC" -eq "standalone" ]; then
	echo idk we ll see
else

	mkdir -p $SYSROOT_DIR/boot/grub
	cat > $SYSROOT_DIR/boot/grub/grub.cfg << EOF
menuentry "$BIN_NAME" {
	"$BOOT_SPEC" /boot/$BIN_NAME
}
EOF

	grub-mkrescue -o $OUT_ISO_PATH $SYSROOT_DIR
fi	
