# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=2

inherit enlightenment

DESCRIPTION="Enlightenment's core event abstraction layer and OS abstraction layer"

IUSE="curl directfb +evas fbcon glib gnutls +inotify opengl sdl ssl static-libs +threads tslib +X xcb xinerama xprint xscreensaver"

RDEPEND="
	>=dev-libs/eina-9999
	glib? ( dev-libs/glib )
	curl? ( net-misc/curl )
	gnutls? ( net-libs/gnutls )
	!gnutls? ( ssl? ( dev-libs/openssl ) )
	evas? (
		>=media-libs/evas-9999[directfb?,fbcon?,opengl?,sdl?,X?,xcb?]
		opengl? ( virtual/opengl )
	)
	directfb? ( >=dev-libs/DirectFB-0.9.16 )
	tslib? ( x11-libs/tslib )
	sdl? ( media-libs/libsdl )
	X? (
		x11-libs/libX11
		x11-libs/libXcomposite
		x11-libs/libXcursor
		x11-libs/libXdamage
		x11-libs/libXext
		x11-libs/libXfixes
		x11-libs/libXi
		x11-libs/libXrender
		x11-libs/libXtst
		xinerama? ( x11-libs/libXinerama x11-libs/libXrandr )
		xprint? ( x11-libs/libXp )
		xscreensaver? ( x11-libs/libXScrnSaver )
	)
	!X? ( xcb? ( x11-libs/xcb-util ) )"
DEPEND="${RDEPEND}"

src_configure() {
	local SSL_FLAGS="" EVAS_FLAGS="" X_FLAGS=""

	if use gnutls; then
		if use ssl; then
			einfo "You have enabled both 'ssl' and 'gnutls', so we will use"
			einfo "gnutls and not openssl for ecore-con support"
		fi
		SSL_FLAGS="
		  --disable-openssl
		  --enable-gnutls
		"
	elif use ssl; then
		SSL_FLAGS="
		  --enable-openssl
		  --disable-gnutls
		"
	else
		SSL_FLAGS="
		  --disable-openssl
		  --disable-gnutls
		"
	fi

	local x_or_xcb=""
	if use X; then
		x_or_xcb="X"
	elif use xcb; then
		x_or_xcb="xcb"
	fi

	if use evas; then

		if use opengl && [[ -z "$x_or_xcb" ]]; then
			ewarn "Ecore/Evas usage of OpenGL requires X11."
			ewarn "Compile dev-libs/ecore with USE=X or xcb."
			ewarn "Compiling without opengl support."
			EVAS_FLAGS+="
				--disable-ecore-evas-software-x11
				--disable-ecore-evas-xrender-x11
				--disable-ecore-evas-software-16-x11
			"
		else
			EVAS_FLAGS+="
				--enable-ecore-evas-software-x11
				--enable-ecore-evas-xrender-x11
				--enable-ecore-evas-software-16-x11
			"
		fi
		EVAS_FLAGS+="
			$(use_enable directfb ecore-evas-directfb)
			$(use_enable fbcon ecore-evas-fb)
			$(use_enable sdl ecore-evas-software-sdl)
			$(use_enable xcb ecore-evas-xrender-xcb)
			$(use_enable opengl ecore-evas-opengl-x11)
		"
	else
		EVAS_FLAGS+="
			--disable-ecore-evas-directfb
			--disable-ecore-evas-fb
			--disable-ecore-evas-software-sdl
			--disable-ecore-evas-software-x11
			--disable-ecore-evas-xrender-x11
			--disable-ecore-evas-software-16-x11
			--disable-ecore-evas-xrender-xcb
			--disable-ecore-evas-opengl-x11
		"
		if use opengl; then
			ewarn "Ecore usage of OpenGL is dependent on media-libs/evas."
			ewarn "Compile dev-libs/ecore with USE=evas."
		fi
	fi

	if use X; then
		if use xcb; then
			ewarn "You have enabled both 'X' and 'xcb', so we will use"
			ewarn "X as it's considered the most stable for ecore-x"
		fi
		X_FLAGS="
		  --enable-ecore-x
		  --disable-ecore-x-xcb
		"

	elif use xcb; then
		X_FLAGS="
		  --enable-ecore-x
		  --enable-ecore-x-xcb
		"
	else
		X_FLAGS="
		  --disable-ecore-x
		  --disable-ecore-x-xcb
		"
	fi

	if [[ ! -z "$x_or_xcb" ]]; then
		X_FLAGS+="
			$(use_enable xinerama ecore-x-xinerama)
			$(use_enable xprint ecore-x-xprint)
			$(use_enable xscreensaver ecore-x-screensaver)
		"
	else
		X_FLAGS+="
			--disable-ecore-x-xinerama
			--disable-ecore-x-xprint
			--disable-ecore-x-screensaver
		"
	fi

	if use tslib && ! use fbcon; then
		ewarn "Ecore just uses tslib for framebuffer input."
		ewarn "Compile dev-libs/ecore with USE=fbcon."
	fi

	MY_ECONF="
	--enable-ecore-con
	--enable-ecore-ipc
	--enable-ecore-file
	--enable-ecore-imf
	--enable-ecore-input
	--disable-ecore-win32
	--disable-ecore-wince
	--disable-ecore-evas-software-gdi
	--disable-ecore-evas-software-ddraw
	--disable-ecore-evas-direct3d
	--disable-ecore-evas-opengl-glew
	--disable-ecore-evas-software-16-ddraw
	--disable-ecore-evas-software-16-wince
	$(use_enable curl)
	$(use_enable directfb ecore-directfb)
	$(use_enable doc)
	$(use_enable evas ecore-evas)
	$(use_enable evas ecore-input-evas)
	$(use_enable evas ecore-imf-evas)
	$(use_enable evas ecore-evas-software-buffer)
	$(use_enable fbcon ecore-fb)
	$(use_enable glib)
	$(use_enable inotify)
	$(use_enable sdl ecore-sdl)
	$(use_enable test tests)
	$(use_enable threads pthread)
	$(use_enable tslib)
	$(use_enable X xim)
	${SSL_FLAGS}
	${EVAS_FLAGS}
	${X_FLAGS}
	"
	enlightenment_src_configure
}
