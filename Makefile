#
#
# Copyright (C) 2004 by Oleg I. Vdovikin <oleg@cs.msu.su>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

ROOT := $(shell (cd .. && pwd -P))
SRC := $(ROOT)/router
export TOP := $(ROOT)/gateway
export KERNEL_DIR := $(ROOT)/linux/linux

BUSYBOX=busybox-1.15.2
DROPBEAR=dropbear-0.52
DNSMASQ=dnsmasq-2.51
P910ND=p910nd-0.93
SAMBA=samba-2.0.10
IPROUTE2=iproute2-2.4.7-now-ss020116-try
#E2FSPROGS=e2fsprogs-1.35
UCDSNMP=ucd-snmp-3.6.2
IPTABLES=iptables-1.3.8
PPP=ppp-2.4.5-pre
RP-PPPOE=rp-pppoe-3.10
ACCEL-PPTP=accel-pptp-git-20091003
PPTP=pptp-1.7.1
LZMA=lzma457
NFSUTILS=nfs-utils-1.0.7
PORTMAP=portmap_4
RADVD=radvd-0.7.3
L2TP=rp-l2tp-0.4
XL2TPD=xl2tpd-1.2.4
IGMPPROXY=igmpproxy-0.1
VSFTPD=vsftpd-2.2.0
UDPXY=udpxy-1.0-Chipmunk-14
NTPCLIENT=ntpclient-2007_365
SCSIIDLE=scsi-idle-2.4.23
LIBUSB=libusb-compat-0.1.3
LIBUSB10=libusb-1.0.3
USBMODESWITCH=usb_modeswitch-1.0.5
MADWIMAX=madwimax-0.1.1

UCLIBC=uClibc-0.9.29

ET=et-4.108.9
WL=wl-4.150.10.29
LIBBCMCRYPTO=libbcmcrypto-3.130.20
WLCONF=wlconf

# tar has --exclude parameter ?
TAR_EXCL_SVN := $(shell tar --exclude .svn -cf - Makefile >/dev/null 2>&1 && echo "--exclude .svn")

PATCHER := $(shell pwd)/patch.sh

define patches_list
    $(shell ls -1 $(1)/[0-9][0-9][0-9]-*.patch 2>/dev/null)
endef

DIFF := LC_ALL=C TZ=UTC0 diff

define make_diff
    (cd $(ROOT) && $(DIFF) $(1) -x'*.o' -x'*.orig' $(2)/$(4) $(3)/$(4) | grep -v "^Files .* differ$$" | grep -v ^Binary.*differ$$) > $(4).diff
    diffstat $(4).diff
endef

OPENWRT_Kernel_Patches:=$(call patches_list,kernel/openwrt)
OPENWRT_Brcm_Patches:=$(call patches_list,kernel/openwrt/brcm)
OUR_Kernel_Patches:=$(call patches_list, kernel)

all: prep custom
	@true

custom:	$(TOP)/.config loader busybox dropbear dnsmasq p910nd samba iproute2 iptables \
	ppp pptp rp-l2tp rp-pppoe accel-pptp xl2tpd \
	nfs-utils portmap radvd ucdsnmp igmpproxy vsftpd udpxy \
	ntpclient bpalogin bridge ez-ipupdate httpd infosvr jpeg-6b lib LPRng \
	misc netconf nvram others rc rcamdmips sendmail \
	scsi-idle libusb usb_modeswitch wimax \
	shared test upnp utils vlan wlconf www rt2460 libbcmcrypto asustrx
	@echo
	@echo Sources prepared for compilation
	@echo

$(TOP):
	@mkdir -p $(TOP)

$(TOP)/Makefile: Makefile.top
	cp $^ $@

prep: $(TOP) $(TOP)/Makefile
	-svnversion 2> /dev/null > $(TOP)/.svnrev

$(TOP)/.config: config shared
	$(MAKE) -C $(KERNEL_DIR) include/linux/version.h
	$(MAKE) -C $(TOP) .config

$(ROOT)/lzma: $(LZMA).tbz2 $(ROOT)/lzma/CPP/7zip/Compress
	@rm -rf $@ && mkdir -p $@
	tar -C $@ -xjf $(LZMA).tbz2
	$(PATCHER) -Z $@ $(LZMA).patch

lzma: $(ROOT)/lzma
	@true

et:
	[ -d $(ROOT)/$(ET).orig ] || mv $(ROOT)/et $(ROOT)/$(ET).orig
	tar -C $(ROOT) -xzf $(ET).tar.gz
	$(PATCHER) -Z $(ROOT)/et $(ET).patch

wl:
	[ -d $(ROOT)/$(WL).orig ] || mv $(ROOT)/wl $(ROOT)/$(WL).orig
	tar -C $(ROOT) -xjf $(WL).tar.bz2

brcm-shared:
	@cd brcm-src && $(PATCHER) -Z $(ROOT) brcm-src-shared.patch brcm-src-include.patch \
		brcm-src-5365.patch brcm-src-5365-robo.patch brcm-src-5354.patch \
		brcm-src-robo-tag.patch 

kernel-mrproper:
	$(MAKE) -C $(KERNEL_DIR) mrproper

kernel-patch:
	@echo Preparing kernel ...
	[ -d $(KERNEL_DIR)/arch/mips/bcm947xx ] || tar -C $(KERNEL_DIR) -xvjf kernel/brcm-boards.tar.bz2
	$(MAKE) -C $(KERNEL_DIR)/arch/mips/bcm947xx/compressed/ clean
	@$(PATCHER) -Z $(KERNEL_DIR) kernel/buildhost.patch
	$(MAKE) -C $(KERNEL_DIR) mrproper
	@echo Patching kernel...
	@$(PATCHER) -Z $(KERNEL_DIR) $(OPENWRT_Kernel_Patches)
	@$(PATCHER) -Z $(KERNEL_DIR) $(OPENWRT_Brcm_Patches)
	@$(PATCHER) -Z $(KERNEL_DIR) $(OUR_Kernel_Patches)

kernel-extra-drivers:
	tar -C $(KERNEL_DIR) -xvjf kernel/drivers/ov51x-1.65-1.12.tar.bz2
	tar -C kernel/drivers/pwc-9.0.2 $(TAR_EXCL_SVN) -cf - . | tar -C $(KERNEL_DIR)/drivers/usb -xf -
	if [ ! -d $(KERNEL_DIR)/fs/fuse ]; then \
	  tar -C $(KERNEL_DIR)/fs -xvjf kernel/drivers/fuse-2.5.3.tar.bz2 fuse-2.5.3/kernel/ \
	   && mv $(KERNEL_DIR)/fs/fuse-2.5.3/kernel $(KERNEL_DIR)/fs/fuse && rmdir $(KERNEL_DIR)/fs/fuse-2.5.3; \
	  $(PATCHER) -Z $(KERNEL_DIR)/fs/fuse kernel/drivers/fuse-2.5.3.patch; \
	fi

kernel: lzma et wl brcm-shared kernel-patch kernel-extra-drivers
	cp kernel/kernel.config $(KERNEL_DIR)/arch/mips/defconfig-bcm947xx

asustrx:
	tar -C $(ROOT) -xjf asustrx.tar.bz2 

$(TOP)/loader: loader/Makefile
	@rm -rf $(TOP)/loader
	tar -C . $(TAR_EXCL_SVN) -cf - loader | tar -C $(TOP) -xf -

loader: $(TOP)/loader
	@true

busybox_Patches := $(call patches_list,busybox)

$(TOP)/busybox: busybox/$(BUSYBOX).tar.bz2
	@rm -rf $(TOP)/$(BUSYBOX) $@
	tar -xjf busybox/$(BUSYBOX).tar.bz2 -C $(TOP)
	mv $(TOP)/$(BUSYBOX)/e2fsprogs/old_e2fsprogs/* $(TOP)/$(BUSYBOX)/e2fsprogs/
	$(PATCHER) -Z $(TOP)/$(BUSYBOX) $(busybox_Patches)
	mkdir -p $(TOP)/$(BUSYBOX)/sysdeps/linux/
	cp busybox/busybox.config $(TOP)/$(BUSYBOX)/sysdeps/linux/defconfig
	chmod a+x $(TOP)/$(BUSYBOX)/testsuite/*.tests
	mv $(TOP)/$(BUSYBOX) $@

busybox: $(TOP)/busybox
	@true

vsftpd_Patches := $(call patches_list,vsftpd)

$(TOP)/vsftpd: vsftpd/$(VSFTPD).tar.gz
	@rm -rf $(TOP)/$(VSFTPD) $@
	tar -xzf vsftpd/$(VSFTPD).tar.gz -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(VSFTPD) $(vsftpd_Patches)
	mv $(TOP)/$(VSFTPD) $@

vsftpd: $(TOP)/vsftpd
	@true

dropbear_Patches := $(call patches_list,dropbear)

$(TOP)/dropbear: dropbear/$(DROPBEAR).tar.bz2
	@rm -rf $(TOP)/$(DROPBEAR) $@
	tar -xjf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(DROPBEAR) $(dropbear_Patches)
	mv $(TOP)/$(DROPBEAR) $@

dropbear: $(TOP)/dropbear
	@true

ucdsnmp_Patches := $(call patches_list,ucd-snmp)

$(TOP)/ucdsnmp: ucd-snmp/$(UCDSNMP).tar.gz
	@rm -rf $(TOP)/$(UCDSNMP) $@
	tar -xzf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(UCDSNMP) $(ucdsnmp_Patches)
	mv $(TOP)/$(UCDSNMP) $@

ucdsnmp: $(TOP)/ucdsnmp
	@true

iproute2_Patches := $(call patches_list,iproute2)

$(TOP)/iproute2: iproute2/$(IPROUTE2).tar.bz2
	@rm -rf $(TOP)/$@
	tar -xjf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/iproute2 $(iproute2_Patches) && touch $@

iproute2: $(TOP)/iproute2
	@true

$(TOP)/dnsmasq: $(DNSMASQ).tar.gz
	@rm -rf $(TOP)/$(DNSMASQ) $@
	tar -xzf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(DNSMASQ) $(DNSMASQ).patch
	mv $(TOP)/$(DNSMASQ) $@ && touch $@

dnsmasq-diff: $(DNSMASQ).tar.gz
	@rm -rf $(TOP)/$(DNSMASQ)
	tar -xzf $^ -C $(TOP)
	-$(MAKE) -C $(TOP)/dnsmasq clean
	-(cd $(TOP) && $(DIFF) -BurpN $(DNSMASQ) dnsmasq) > $(DNSMASQ).patch

dnsmasq: $(TOP)/dnsmasq
	@true

$(TOP)/p910nd: $(P910ND).tar.bz2
	@rm -rf $(TOP)/$(P910ND) $@
	tar -xjf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(P910ND) $(P910ND).patch
	mv $(TOP)/$(P910ND) $@ && touch $@

p910nd-diff:
	@rm -rf $(TOP)/$(P910ND)
	tar -xjf $(P910ND).tar.bz2 -C $(TOP)
	-rm -f $(TOP)/p910nd/p910nd
	-cd $(TOP) && $(DIFF) -BurpN $(P910ND) p910nd > $(P910ND).patch

p910nd: $(TOP)/p910nd
	@true

samba_Patches := $(call patches_list,samba)

$(TOP)/samba: samba/$(SAMBA).tar.bz2
	@rm -rf $(TOP)/$(SAMBA) $@
	tar -xjf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(SAMBA) $(samba_Patches)
	tar -xzvf samba/$(SAMBA)-codepages.tar.gz -C $(TOP)/$(SAMBA)
	mv $(TOP)/$(SAMBA) $@

samba: $(TOP)/samba
	@true

iptables_Patches := $(call patches_list,iptables)

$(TOP)/iptables: iptables/$(IPTABLES).tar.bz2
	@rm -rf $(TOP)/$(IPTABLES) $@
	tar -xjf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(IPTABLES) $(iptables_Patches)
	chmod a+x $(TOP)/$(IPTABLES)/extensions/.*-test $(TOP)/$(IPTABLES)/extensions/.*-test6
	mv $(TOP)/$(IPTABLES) $@ && touch $@

iptables: $(TOP)/iptables
	@true

$(TOP)/nfs-utils:
	@rm -rf $(TOP)/$(NFSUTILS) $@
	tar -xzf $(NFSUTILS).tar.gz -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(NFSUTILS) $(NFSUTILS).patch $(NFSUTILS)-libnfs.patch 
	mv $(TOP)/$(NFSUTILS) $@

nfs-utils: $(TOP)/nfs-utils
	@true

$(TOP)/portmap: $(PORTMAP).tar.gz
	@rm -rf $(TOP)/$(PORTMAP) $@
	tar -xzf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(PORTMAP) $(PORTMAP).patch
	mv $(TOP)/$(PORTMAP) $@

portmap: $(TOP)/portmap
	@true

$(TOP)/radvd:
	@rm -rf $(TOP)/$(RADVD) $@
	tar -xzf $(RADVD).tar.gz -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(RADVD) $(RADVD).patch
	mv $(TOP)/$(RADVD) $@

radvd: $(TOP)/radvd
	@true

rc_Patches := $(call patches_list,rc)

$(TOP)/rc/Makefile:
	tar -C $(SRC) -cf - rc | tar -C $(TOP) -xf -
	$(PATCHER) -Z $(TOP) $(rc_Patches)
	$(MAKE) -C $(TOP)/rc clean

rc: $(TOP)/rc/Makefile
	@true

ppp_Patches := $(call patches_list,ppp)

$(TOP)/ppp: ppp/$(PPP).tar.bz2
	@rm -rf $(TOP)/$(PPP) $@
	tar -xjf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(PPP) $(ppp_Patches)
	mv $(TOP)/$(PPP) $@ && touch $@

ppp: $(TOP)/ppp
	@true

rp-l2tp_Patches := $(call patches_list,rp-l2tp)

$(TOP)/rp-l2tp:
	@rm -rf $(TOP)/$(L2TP) $@
	tar -xzf rp-l2tp/$(L2TP).tar.gz -C $(TOP)
	$(PATCHER) -Z $(TOP)/$($L2TP) $(rp-l2tp_Patches)
	mv $(TOP)/$(L2TP) $@

rp-l2tp: $(TOP)/rp-l2tp
	@true

xl2tpd_Patches := $(call patches_list,xl2tpd)

$(TOP)/xl2tpd:
	@rm -rf $(TOP)/$(XL2TPD) $@
	tar -xzf xl2tpd/$(XL2TPD).tar.gz -C $(TOP)
	$(PATCHER) -Z $(TOP)/$($XL2TPD) $(xl2tpd_Patches)
	mv $(TOP)/$(XL2TPD) $@

xl2tpd: $(TOP)/xl2tpd
	@true

rp-pppoe_Patches := $(call patches_list,rp-pppoe)

$(TOP)/rp-pppoe: rp-pppoe/$(RP-PPPOE).tar.gz
	@rm -rf $(TOP)/$(RP-PPPOE) $@
	tar -xzf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(RP-PPPOE) $(rp-pppoe_Patches)
	mv $(TOP)/$(RP-PPPOE) $@ && touch $@

rp-pppoe: $(TOP)/rp-pppoe
	@true

accel-pptp_Patches := $(call patches_list,accel-pptp)

$(TOP)/accel-pptp: accel-pptp/$(ACCEL-PPTP).tar.bz2
	@rm -rf $(TOP)/$(ACCEL-PPTP) $@
	tar -xjf $^ -C $(TOP)
	rm -rf $(TOP)/$(ACCEL-PPTP)/pppd_plugin/src/pppd
	ln -s $(TOP)/ppp/pppd $(TOP)/$(ACCEL-PPTP)/pppd_plugin/src/pppd
	$(PATCHER) -Z $(TOP)/$(ACCEL-PPTP) $(accel-pptp_Patches)
	mv $(TOP)/$(ACCEL-PPTP) $@ && touch $@
	touch $@

accel-pptp: $(TOP)/accel-pptp
	@true

igmpproxy_Patches := $(call patches_list,igmpproxy)

$(TOP)/igmpproxy: igmpproxy/$(IGMPPROXY).tar.gz
	@rm -rf $(TOP)/igmpproxy
	tar -xzf igmpproxy/$(IGMPPROXY).tar.gz -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(IGMPPROXY) $(igmpproxy_Patches)
	mv $(TOP)/$(IGMPPROXY) $@ && touch $@

igmpproxy: $(TOP)/igmpproxy
	@true

pptp_Patches := $(call patches_list,pptp)

$(TOP)/pptp: pptp/$(PPTP).tar.gz
	@rm -rf $(TOP)/$(PPTP) $@
	tar -xzf $^ -C $(TOP)
	$(PATCHER) -Z $(TOP)/$(PPTP) $(pptp_Patches)
	mv $(TOP)/$(PPTP) $@ && touch $@

pptp: $(TOP)/pptp
	@true

$(TOP)/udpxy: $(UDPXY).tgz
	@rm -rf $(TOP)/udpxy-wl $@
	tar -xzf $^ -C $(TOP)
	[ ! -f $(UDPXY).patch ] || $(PATCHER) -Z $(TOP)/$(UDPXY) $(UDPXY).patch
	mv $(TOP)/$(UDPXY) $@ && touch $@

udpxy: $(TOP)/udpxy
	@true

$(TOP)/ntpclient: $(NTPCLIENT).tar.bz2
	@rm -rf $(TOP)/$(NTPCLIENT) $@
	tar -xjf $^ -C $(TOP)
	[ ! -f $(NTPCLIENT).patch ] || $(PATCHER) -Z $(TOP)/$(NTPCLIENT) $(NTPCLIENT).patch
	mv $(TOP)/$(NTPCLIENT) $@ && touch $@

ntpclient: $(TOP)/ntpclient
	@true

ez-ipupdate_Patches := $(call patches_list,ez-ipupdate)

$(TOP)/ez-ipupdate: $(TOP)/ez-ipupdate/Makefile.in
	tar -C $(SRC) -cf - ez-ipupdate | tar -C $(TOP) -xf -
	$(PATCHER) -Z $(TOP) $(ez-ipupdate_Patches)
	$(MAKE) -C $@ clean

ez-ipupdate: $(TOP)/ez-ipupdate
	@true

$(TOP)/scsi-idle: $(SCSIIDLE).tar.gz
	@rm -rf $(TOP)/$(SCSIIDLE) $@
	tar -xzf $^ -C $(TOP)
	[ ! -f $(SCSIIDLE).patch ] || $(PATCHER) -Z $(TOP)/$(SCSIIDLE) $(SCSIIDLE).patch
	mv $(TOP)/$(SCSIIDLE) $@ && touch $@

scsi-idle: $(TOP)/scsi-idle
	@true

$(TOP)/libusb: libusb/$(LIBUSB).tar.bz2
	@rm -rf $(TOP)/$(LIBUSB) $@
	tar -jxf $^ -C $(TOP)
	[ ! -f libusb/$(LIBUSB).patch ] || $(PATCHER) -Z $(TOP)/$(LIBUSB) libusb/$(LIBUSB).patch
	mv $(TOP)/$(LIBUSB) $@ && touch $@

libusb: $(TOP)/libusb10 $(TOP)/libusb
	@true

$(TOP)/libusb10: libusb/$(LIBUSB10).tar.bz2
	@rm -rf $(TOP)/$(LIBUSB10) $@
	tar -jxf $^ -C $(TOP)
	[ ! -f libusb/$(LIBUSB10).patch ] || $(PATCHER) -Z $(TOP)/$(LIBUSB10) libusb/$(LIBUSB10).patch
	mv $(TOP)/$(LIBUSB10) $@ && touch $@

$(TOP)/usb_modeswitch: usb_modeswitch/$(USBMODESWITCH).tar.bz2
	rm -rf $(TOP)/$(USBMODESWITCH) $@
	tar -jxf $^ -C $(TOP)
	cp -pf usb_modeswitch/usb_modeswitch.conf $(TOP)/$(USBMODESWITCH)/
	[ ! -f usb_modeswitch/$(USBMODESWITCH).patch ] || \
		$(PATCHER) -Z $(TOP)/$(USBMODESWITCH) usb_modeswitch/$(USBMODESWITCH).patch
	$(MAKE) -C $(TOP)/$(USBMODESWITCH) clean
	mv $(TOP)/$(USBMODESWITCH) $@ && touch $@

usb_modeswitch: $(TOP)/usb_modeswitch
	@true

$(TOP)/madwimax: wimax/$(MADWIMAX).tar.gz
	rm -rf $(TOP)/$(MADWIMAX) $@
	tar -zxf $^ -C $(TOP)
	[ ! -f wimax/$(MADWIMAX).patch ] || \
		$(PATCHER) -Z $(TOP)/$(MADWIMAX) wimax/$(MADWIMAX).patch
	mv $(TOP)/$(MADWIMAX) $@ && touch $@

wimax: $(TOP)/madwimax
	@true

$(TOP)/others:
	tar -C $(SRC) -cf - others | tar -C $(TOP) -xf -
	tar -C . $(TAR_EXCL_SVN) -cf - others | tar -C $(TOP) -xf -
	$(PATCHER) -Z $(TOP) others.diff

others: $(TOP)/others

libbcmcrypto: $(LIBBCMCRYPTO).tar.gz
	tar -zxf $^ -C $(TOP)
	$(PATCHER) $(TOP)/libbcmcrypto $(LIBBCMCRYPTO).patch

wlconf: $(WLCONF).tar.gz
	tar -zxf $^ -C $(TOP)

upnp:
	[ ! -d $(SRC)/../tools/$@ ] || [ -d $(TOP)/$@ ] || \
		tar -C $(SRC)/../tools -cf - $@ | tar -C $(TOP) -xf -
	[ ! -f $@.diff ] || $(PATCHER) -Z $(TOP) $@.diff

upnp-diff:
	$(call make_diff,-BurpN,tools,gateway,upnp)

$(TOP)/www:
	[ ! -d $(SRC)/www ] || [ -d $@ ] || \
		tar -C $(SRC) -cf - www/asus/web_asus_en \
		www/asus/Makefile www/asus/mkweb www/asus/pages.mk www/asus/notin2MB \
		| tar -C $(TOP) -xf -

www: $(TOP)/www www/pages.diff www/common.diff
	$(PATCHER) -Z $(TOP) www/pages.diff www/common.diff
	chmod a+x $(TOP)/www/asus/remccoms2.sh
	cp www/netcam_mfc_activeX.cab $(TOP)/www/asus/web_asus_en/
	cp www/iBox_title_all.jpg $(TOP)/www/asus/web_asus_en/graph/
	cp www/iBox_title_all_HDD.jpg $(TOP)/www/asus/web_asus_en/graph/
	cp www/iBox_title_all_550g.jpg $(TOP)/www/asus/web_asus_en/graph/

www-diff:
	(cd .. && $(DIFF) -BurN router/www/asus/web_asus_en gateway/www/asus/web_asus_en | grep -v ^Binary.*differ$$) > www/pages.diff
	(cd .. && $(DIFF) -BuN router/www/asus gateway/www/asus | grep -v ^Binary.*differ$$ | grep -v "^Common subdirectories: .*$$") > www/common.diff
	diffstat www/pages.diff

shared-diff:
	$(call make_diff,-BurpN -xbcmconfig.h,router,gateway,shared)

%:
	[ ! -d $(SRC)/$* ] || [ -d $(TOP)/$* ] || \
		tar -C $(SRC) -cf - $* | tar -C $(TOP) -xf -
	[ ! -f $*.diff ] || $(PATCHER) -Z $(TOP) $*.diff
	[ ! -f $*.patch ] || patch -d $(TOP) -d $* -p1 --no-backup-if-mismatch -Z < $*.patch
	[ ! -f $(TOP)/$*/Makefile ] || $(MAKE) -C $(TOP)/$* clean

%-diff:
	[ -d $(SRC)/$* ] || [ -d $(TOP)/$* ] && \
	    $(call make_diff,-BurpN,router,gateway,$*)

%-diff-simple:
	[ -d $(SRC)/$* ] || [ -d $(TOP)/$* ] && \
	    $(call make_diff,-BurN,router,gateway,$*)

.PHONY: custom kernel kernel-patch kernel-extra-drivers brcm-shared www \
	busybox dropbear iptables others
