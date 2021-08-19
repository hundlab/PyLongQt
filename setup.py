#from setuptools import setup
from glob import glob
from skbuild import setup

#ext_modules = [
#    Pybind11Extension(
#        "PyLongQt",
#        sorted(glob("src/*.cpp")),
#    ),
#] 
#
#
with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name='PyLongQt-hundlab',
    version='0.5',
    license='GNU GPL v3.0',
    author='Thomas Hund',
    author_email='hund.1@osu.edu',
    description="Python bindings for LongQt",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/hundlab/PyLongQt",
    project_urls={
        "Documentation": "https://hundlab.github.io/PyLongQt",
    },
#    ext_modules=ext_modules,
    packages=['PyLongQt'],
    package_dir={'PyLongQt': 'src'},
    cmake_install_dir='src',
#    package_data={'': ["*.so"]},
#    include_package_data=True,

#    ext_modules=[CMakeExtension('PyLongQt')],
#    cmdclass={
#        'build_ext': build_ext,
#    }

#    package_data={'PyLongQt': ['PyLongQt.cpython-38-x86_64-linux-gnu.so']},
#    cmdclass={'install': install},
)
