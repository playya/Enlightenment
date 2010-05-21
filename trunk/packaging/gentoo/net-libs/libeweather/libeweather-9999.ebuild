# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
E_NO_NLS="1"
E_NO_DOC="1"
ESVN_SUB_PROJECT="PROTO"
inherit efl

DESCRIPTION="Enlightenment's weather information fetching and parsing framework"
IUSE=""

LICENSE="LGPL-2.1"

RDEPEND="
	>=dev-libs/ecore-9999[curl]
	>=media-libs/edje-9999"
DEPEND="${RDEPEND}"
