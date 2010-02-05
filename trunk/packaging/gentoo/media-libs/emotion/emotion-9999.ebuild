# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
E_NO_NLS="1"
inherit efl

DESCRIPTION="Enlightenment's (Ecore/Evas) video integration."
HOMEPAGE="http://trac.enlightenment.org/e/wiki/Emotion"

# vlc support is buggy, do not even expose it here
IUSE="gstreamer xine static-modules"

# TODO: remove edje dependency as soon as emotion is fixed to not build its test
RDEPEND="
	>=dev-libs/eina-9999
	>=dev-libs/ecore-9999
	>=media-libs/evas-9999
	>=media-libs/edje-9999
	xine? ( >=media-libs/xine-lib-1.1.1 )
	gstreamer? (
		=media-libs/gstreamer-0.10*
		=media-libs/gst-plugins-good-0.10*
		=media-plugins/gst-plugins-ffmpeg-0.10*
	)"
DEPEND="${RDEPEND}"


src_compile() {
	if ! use xine && ! use gstreamer; then
		die "Emotion needs at least one media system to be useful!"
		die "Compile media-libs/emotion with USE=xine or gstreamer."
	fi

	if use static-modules; then
		MODULE_ARGUMENT="static"
	else
		MODULE_ARGUMENT="yes"
	fi

	export MY_ECONF="
	  ${MY_ECONF}
	  $(use_enable xine xine $MODULE_ARGUMENT)
	  $(use_enable gstreamer gstreamer $MODULE_ARGUMENT)
	"

	if use gstreamer; then
		addpredict "/root/.gconfd"
		addpredict "/root/.gconf"
	fi

	efl_src_compile
}
