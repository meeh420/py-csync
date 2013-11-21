import sys
sys.path.insert (0, 'build/lib.linux-x86_64-2.7/') # @todo don't hardcode
import csync

print '=== BEGIN TEST ==='

#cs = csync.CSync ("/tmp/check_csync1", "dummy://foo/bar")
#cs = csync.Create ("/tmp/check_csync1", "dummy://foo/bar")
#cs.set_config_dir ("/tmp/check_csync")

#def auth_callback (*args):
#    print map(type, args)  # @todo bool not int?
#    print '\tnum_args = ' + len(args)
#    return 'password'
#    return None     # error!

import getpass
def auth_callback (prompt, echo, verify):
    return getpass.getpass (prompt)
    # Note: getpass() raises EOFError on ctrl-d

cs = csync.CSync ("testdir", "sftp://srv1/home/torkel/foobar")
cs.set_auth_callback (lambda prompt,u1,u2: getpass.getpass(prompt))
cs.init()

#cs.commit()



"""
#obj = csync.create ('/tmp/a', '/tmp/b')
#obj = csync.create (local = '/tmp/a', remote = '/tmp/b')
obj = csync.CSync (sys.argv[1], sys.argv[2])

obj.disable_statedb()
obj.init()

if obj.is_statedb_disabled():
    print 'statedb is disabled'
else:
    print 'statedb: ' + obj.get_statedb_file()

# if (arguments.create_statedb) {
#     csync_set_status(csync, 0xFFFF);

obj.walk_local_tree (lambda: 0, 0)

obj.update()
obj.propagate()
#obj.reconcile()

print '- The End -'
"""


# TODO
# segfault. report error instead
#obj = csync.CSync()

# python: csyncmodule.c:72: _py_csync_dealloc: Assertion `self->ctx' failed.
#obj = csync.CSync (xlocal = '/tmp/a', remote = '/tmp/b')

# how does it work?
#obj.add_exclude_list ('/tmp/a/ignore')

#print csync.status_ok()
#print type(csync.status_ok())
