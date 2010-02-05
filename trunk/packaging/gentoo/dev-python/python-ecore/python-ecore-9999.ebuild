# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
E_CYTHON="1"
E_NO_NLS="1"
E_NO_DOC="2"
ESVN_SUB_PROJECT="BINDINGS/python"

inherit efl

DESCRIPTION="Python bindings for Ecore"
IUSE="+evas +X +xscreensaver examples"

RDEPEND=">=dev-libs/ecore-9999[evas?,X?,xscreensaver?]"

# python-evas is just required to build as it includes some useful header files
DEPEND="
	>=dev-python/python-evas-9999
	${RDEPEND}"

src_compile() {
	if use evas; then
		export ECORE_BUILD_EVAS=1
	else
		export ECORE_BUILD_EVAS=0
	fi

	if use X; then
		export ECORE_BUILD_X=1

		if use xscreensaver; then
			export ECORE_BUILD_XSCREENSAVER=1
		else
			export ECORE_BUILD_XSCREENSAVER=0
		fi
	else
		export ECORE_BUILD_X=0

		if use xscreensaver; then
			ewarn "USE=xscreensaver has no meaning without X use flag!"
		fi
	fi

	efl_src_compile
}

src_install() {
	if use evas; then
		export ECORE_BUILD_EVAS=1
	else
		export ECORE_BUILD_EVAS=0
	fi

	if use evas; then
		export ECORE_BUILD_EVAS=1
	else
		export ECORE_BUILD_EVAS=0
	fi

	if use X; then
		export ECORE_BUILD_X=1

		if use xscreensaver; then
			export ECORE_BUILD_XSCREENSAVER=1
		else
			export ECORE_BUILD_XSCREENSAVER=0
		fi
	else
		export ECORE_BUILD_X=0
	fi

	efl_src_install
}
