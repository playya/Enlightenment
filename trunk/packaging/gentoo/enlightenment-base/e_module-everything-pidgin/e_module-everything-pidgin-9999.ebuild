# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
ESVN_SUB_PROJECT="E-MODULES-EXTRA"
ESVN_URI_APPEND="${PN#e_module-}"
E_NO_DOC="1"
inherit efl

DESCRIPTION="everything-pidgin module for enlightenment"
HOMEPAGE="http://www.enlightenment.org"
SRC_URI=""

LICENSE="BSD"
SLOT="0"
KEYWORDS=""

IUSE=""

RDEPEND=">=x11-wm/enlightenment-9999[everything]
	net-im/pidgin"
DEPEND="${RDEPEND}"
