import sys
sys.path.insert (0, 'build/lib.linux-x86_64-2.7/') # @todo don't hardcode
import csync

print '=== BEGIN TEST ==='

#print csync.CONF_DIR
#print csync.VERSION
print csync.version(0)

#o1 = csync.CSync()

#print csync.version ()
#print csync.version (10753)
#print csync.version ('10753')
#print csync.version (None)

#o = csync.create ('/tmp/a', '/tmp/b')
#o = csync.create (local = '/tmp/a', remote = '/tmp/b')

# segfault. report error instead
#obj = csync.CSync()

#obj = csync.CSync (sys.argv[1], sys.argv[2])
obj = csync.CSync ('/tmp/a', '/tmp/b')
#obj = csync.CSync (local = '/tmp/a', remote = '/tmp/b')
#print dir(obj)

# python: csyncmodule.c:72: _py_csync_dealloc: Assertion `self->ctx' failed.
#obj = csync.CSync (xlocal = '/tmp/a', remote = '/tmp/b')

# how does it work?
obj.add_exclude_list ('/tmp/a/ignore')

obj.init()
obj.update()
obj.propagate()

#print csync.status_ok()
#print type(csync.status_ok())
