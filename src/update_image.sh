#!/bin/bash

mkdir iso
mkdir -p iso/boot/grub

cp kernel iso/
cp grub.cfg.example iso/boot/grub/grub.cfg

grub2-mkrescue --output=kernel.iso iso

rm -rf iso
