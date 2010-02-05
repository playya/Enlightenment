# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
ESVN_SUB_PROJECT="PROTO"
E_NO_NLS="1"
E_NO_DOC="1"
inherit efl

DESCRIPTION="Enlightenment way to exchange contributed stuff"
IUSE=""

DEPEND="
	>=dev-libs/eina-9999
	>=dev-libs/ecore-9999[curl,evas]
	>=media-libs/edje-9999
	dev-libs/libxml2
"

RDEPEND="${DEPEND}"


src_compile() {
	export MY_ECONF="
	  ${MY_ECONF}
	  --disable-etk
	  --disable-ewl
	"

	efl_src_compile
}
