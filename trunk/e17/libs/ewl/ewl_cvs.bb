DESCRIPTION = "The Enlightened Widget Library, \
a simple-to-use general purpose widget library based on the Evas canvas."
SECTION = "libs"
DEPENDS = "edje evas ecore etox"
PV = "${CVSDATE}"
PR = "r0"

#SRC_URI = "cvs://anonymous@cvs.sourceforge.net/cvsroot/enlightenment;module=e17/libs/ewl${CVS_METHOD}"
#S = "${WORKDIR}/ewl"

addtask prepsources after do_fetch before do_unpack

SRC_URI = "file://./"
S = "${WORKDIR}/ewl"

inherit autotools binconfig

do_stage () {
	oe_libinstall -C src libewl ${STAGING_LIBDIR}/
	install -m 0644 ${S}/src/Ewl.h ${STAGING_INCDIR}/
}

PACKAGES += "ewl-examples"

FILES_${PN} = "${libdir}/libewl*.so*"
FILES_${PN}-dev += "${bindir}/ewl-config ${libdir}/pkgconfig"
FILES_${PN}-examples = "${bindir}/ewl* ${bindir}/edje_ls ${datadir}"

