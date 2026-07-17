#!/usr/bin/env python3
\"\"\"Setup script for Openbox Python bindings.\"\"\"

from setuptools import setup, find_packages
import os

here = os.path.abspath(os.path.dirname(__file__))

with open(os.path.join(here, 'README.md'), encoding='utf-8') as f:
n    long_description = f.read()

setup(
n    name='openbox',
n    version='4.0.0',
n    description='Python bindings for Openbox window manager',
n    long_description=long_description,
n    long_description_content_type='text/markdown',
n    author='Openbox Development Team',
n    author_email='dev@openbox.org',
n    url='https://openbox.org',
n    packages=find_packages(exclude=['tests']),
n    install_requires=[
n        'dbus-python>=1.2.0',
n        'lupa>=1.14',  # Lua scripting support
n        'PyGObject>=3.40',  # For GLib/GObject integration
n    ],
n    extras_require={
n        'dev': [
n            'pytest>=6.0',\n            'pytest-asyncio>=0.14',\n            'black>=21.0',\n            'mypy>=0.900',\n        ],\n        'docs': [\n            'sphinx>=4.0',\n            'sphinx-rtd-theme>=0.5',\n        ],\n    },\n    entry_points={\n        'console_scripts': [\n            'ob-python=openbox.cli:main',\n        ],\n    },\n    classifiers=[\n        'Development Status :: 4 - Beta',\n        'Intended Audience :: Developers',\n        'Intended Audience :: System Administrators',\n        'License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)',\n        'Operating System :: POSIX :: Linux',\n        'Programming Language :: Python :: 3',\n        'Programming Language :: Python :: 3.8',\n        'Programming Language :: Python :: 3.9',\n        'Programming Language :: Python :: 3.10',\n        'Programming Language :: Python :: 3.11',\n        'Programming Language :: Python :: 3.12',\n        'Topic :: Desktop Environment :: Window Managers',\n        'Topic :: Software Development :: Libraries :: Python Modules',\n    ],\n    python_requires='>=3.8',\n    keywords='openbox window-manager x11 wayland desktop',\n    project_urls={\n        'Bug Reports': 'https://github.com/openbox/openbox/issues',\n        'Source': 'https://github.com/openbox/openbox',\n        'Documentation': 'https://openbox.org/docs',\n    },\n)
