# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
E_NO_NLS="1"
inherit efl

DESCRIPTION="Enlightenment's thumbnailing library"
HOMEPAGE="http://trac.enlightenment.org/e/wiki/Ethumb"

LICENSE="LGPL-3"

IUSE="+dbus emotion epdf"

RDEPEND="
	>=dev-libs/eina-9999
	>=dev-libs/ecore-9999[evas]
	>=media-libs/edje-9999
	>=media-libs/evas-9999
	dbus? ( >=dev-libs/e_dbus-9999 )
	emotion? ( >=media-libs/emotion-9999 )
	epdf? ( >=app-text/epdf-9999 )"

DEPEND="${RDEPEND}"

src_compile() {
	local DEBUG_FLAGS=""

	if ! use debug; then
		DEBUG_FLAGS="--with-internal-maximum-log-level=2"
	fi

	export MY_ECONF="
	  ${MY_ECONF}
	  --with-dbus-services=/etc/dbus-1/session.d
	  $(use_enable dbus ethumbd)
	  $(use_enable emotion)
	  $(use_enable epdf)
	  ${DEBUG_FLAGS}"

	efl_src_compile
}
