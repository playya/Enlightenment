DESCRIPTION = "Enlightenment 0.17.0 snapshot release"
HOMEPAGE = "http://www.enlightenment.org"
MAINTAINER = "Carsten Haitzler (Rasterman) <raster@rasterman.com>"
SECTION = "e/windowmanager"
PRIORITY = "optional"
DEPENDS = "ecore edje eet embryo evas imlib2 embryo-native edje-native imlib2-native"
PV = "0.17.0_pre10"
PR = "1"

do_prepsources () {
  make clean distclean || true
}
addtask prepsources after do_fetch before do_unpack
SRC_URI = "file://./"
S = "${WORKDIR}/e"

inherit autotools pkgconfig binconfig

export EET_CONFIG = "${STAGING_BINDIR}/eet-config"
export EVAS_CONFIG = "${STAGING_BINDIR}/evas-config"
export ECORE_CONFIG = "${STAGING_BINDIR}/ecore-config"
export EMBRYO_CONFIG = "${STAGING_BINDIR}/embryo-config"
export EDJE_CONFIG = "${STAGING_BINDIR}/edje-config"

EXTRA_OECONF = "--with-profile=HIRES_PDA \
                --with-edje-cc=/usr/local/bin/edje_cc"

FILES_${PN} = "${bindir}/* ${libdir}/* ${datadir}"
