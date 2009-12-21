import sys
import os

if not os.path.exists("elementary/elementary.c_elementary.c"):
    try:
        import Cython
    except ImportError:
        raise SystemExit("You need Cython -- http://cython.org/")
    try:
        import Pyrex
    except ImportError:
        raise SystemExit(
            "You need Pyrex -- "
            "http://www.cosc.canterbury.ac.nz/greg.ewing/python/Pyrex/")

from distutils.core import setup
from setuptools import find_packages
from distutils.extension import Extension
from Cython.Distutils import build_ext
import commands

debug = False

class elementary_build_ext(build_ext):
    def finalize_options(self):
        build_ext.finalize_options(self)
        self.include_dirs.insert(0,'include')
        self.pyrex_include_dirs.extend(self.include_dirs)

def pkgconfig(*packages, **kw):
    flag_map = {'-I': 'include_dirs', '-L': 'library_dirs', '-l': 'libraries'}
    pkgs = ' '.join(packages)
    cmdline = 'pkg-config --libs --cflags %s' % pkgs

    status, output = commands.getstatusoutput(cmdline)
    if status != 0:
        raise ValueError("could not find pkg-config module: %s" % pkgs)

    for token in output.split():
        if flag_map.get(token[:2]):
            kw.setdefault(flag_map.get(token[:2]), []).append(token[2:])
        elif token.startswith("-Wl,"):
            kw.setdefault("extra_link_args", []).append(token)
        else:
            kw.setdefault("extra_compile_args", []).append(token)
    return kw

depends = ['include/elementary/c_elementary.pxd']

for root, dirs, files in os.walk('elementary'):
    for file in files:
        if file.endswith('.pxi'):
            depends.append('elementary/' + file)
            
if not debug:
    elementary_mod = Extension('elementary.c_elementary',
                       sources=['elementary/elementary.c_elementary.pyx'],
                       depends=depends,
                       **pkgconfig('"elementary"'))
else:
    elementary_mod = Extension('elementary.c_elementary',
                       sources=['elementary/elementary.c_elementary.pyx'],
                       depends=depends,
                       extra_compile_args=["-g"],
                       extra_link_args=["-g"],
                       **pkgconfig('"elementary"'))

headers = ['include/elementary/c_elementary.pxd',
           'include/elementary/__init__.py',
          ]

setup(
    name = 'python-elementary',
    version = '0.1',
    license = 'LGPL',
    author = 'Simon Busch',
    author_email = 'morphis@gravedo.de',
    url='http://www.freeesmartphone.org',
    description = 'Python bindings for Elementary',
    long_description = '',
    keywords = 'wrapper bindings ui elementary graphics',
    packages = find_packages(),
    headers = headers,
 #   classifiers = 
 #   packages = 
 #   install_requires = ['elementary','python-evas>=0.2.1'],
 #   setup_requires = ['elemtnary','python-evas>=0.2.1'],
 #   headers = 
 #   zip_safe=False,
    cmdclass = {"build_ext": elementary_build_ext},
    ext_modules = [elementary_mod]
)
