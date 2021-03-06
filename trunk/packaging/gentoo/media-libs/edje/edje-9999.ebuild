# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/media-libs/edje/edje-9999.ebuild,v 1.6 2006/07/16 05:29:42 vapier Exp $

inherit enlightenment

EAPI="2"

DESCRIPTION="graphical layout and animation library"
HOMEPAGE="http://www.enlightenment.org/pages/edje.html"

IUSE="debug cache static-libs vim-syntax"

DEPEND="dev-lang/lua
	>=dev-libs/eet-9999
	>=dev-libs/eina-9999
	>=dev-libs/embryo-9999
	>=media-libs/evas-9999[eet]
	>=dev-libs/ecore-9999"
RDEPEND=${DEPEND}

src_compile() {
	export MY_ECONF="
		$(use_enable cache edje-program-cache)
		$(use_enable cache edje-calc-cache)
		$(use_enable !debug amalgamation)
		$(use_with vim-syntax vim /usr/share/vim)
	"
	enlightenment_src_compile
}

src_install() {
	if use vim-syntax ; then
		insinto /usr/share/vim/vimfiles/syntax/
		doins data/edc.vim edc.vim
	fi

	dodoc utils/gimp-edje-export.py
	dodoc utils/inkscape2edc

	enlightenment_src_install
}
