from distutils.core import setup, Extension

setup (name        = 'py-csync',
       version     = '0.0.1alpha',     # or match libcsync version?
       description = 'blah',
       ext_modules =
       [
           Extension ('csync',
                      sources   = ['csyncmodule.c'],
                      libraries = ['csync'],
           ),
       ]
)
