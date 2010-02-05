# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
E_CYTHON="1"
E_NO_NLS="1"
E_NO_DOC="2"
ESVN_SUB_PROJECT="BINDINGS/python"

inherit efl

DESCRIPTION="Python bindings for Emotion"
IUSE="examples"

RDEPEND=">=media-libs/emotion-9999"

# python-evas is just required to build as it includes some useful header files
DEPEND="
	>=dev-python/python-evas-9999
	${RDEPEND}"
