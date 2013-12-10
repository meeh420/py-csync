# Examples of using the Python libcsync bindings.

It's possible to run these files without installing py-csync.
But then `PYTHONPATH` must be correctly set. On Linux this can be
done like this:

    cd /path/to/py-csync
    tmp=`find build/ -name \*.so`
    export PYTHONPATH=`pwd`/`dirname $tmp`
