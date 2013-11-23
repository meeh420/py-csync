import sys
sys.path.insert (0, 'build/lib.linux-x86_64-2.7/') # @todo don't hardcode
import csync

print '=== BEGIN TEST ==='

#cs = csync.CSync ("testdir", "sftp://srv1/home/torkel/foobar")
#cs = csync.CSync ("/tmp/check_csync1", "dummy://foo/bar")
#cs = csync.Create ("/tmp/check_csync1", "dummy://foo/bar")
#cs.set_config_dir ("/tmp/check_csync")


## Walk local/remote file tree

# (path, modtime, uid, gid, mode, type, instruction)
# @todo document type.
# @todo better to call with dict than tuple?
# @todo expose csync instruction enum
import pwd
from datetime import datetime
def treewalk_visitor (*args):
    filename = args[0]
    #time = time.ctime (args[1])
    modtime = datetime.fromtimestamp(args[1]).strftime("%F %H:%M")
    user = pwd.getpwuid (args[2])[0]    # slow!
    mode = args[4] & 0b111111111
    print oct(mode) + "\t" + user + "\t" + modtime + "   " + filename


import os
os.system ('mkdir -p .tmp')

cs = csync.CSync ("testdir", "tmp")
#cs = csync.CSync (".", ".tmp")
cs.init()
cs.update()
# @note walk_*_tree must be called after 'update' and before 'propagate'
cs.walk_local_tree (treewalk_visitor, 0xff)
#cs.walk_remote_tree (treewalk_visitor, 0xff)
#cs.propagate()
#cs.commit()

os.system ('rm -rf .tmp')




## Auth callback

#def auth_callback (*args):
#    print map(type, args)  # @todo bool not int?
#    print '\tnum_args = ' + len(args)
#    return 'password'
#    return None     # error!

import getpass
def auth_callback (prompt, echo, verify):
    return getpass.getpass (prompt)
    # Note: getpass() raises EOFError on ctrl-d

#cs = csync.CSync ("testdir", "sftp://srv1/home/torkel/foobar")
#cs.set_auth_callback (lambda prompt,u1,u2: getpass.getpass(prompt))
#cs.init()





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
