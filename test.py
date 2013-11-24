import sys
sys.path.insert (0, 'build/lib.linux-x86_64-2.7/') # @todo don't hardcode
import csync

print '=== BEGIN TEST ==='

#cs = csync.CSync ("testdir", "sftp://srv1/home/torkel/foobar")
#cs = csync.CSync ("/tmp/check_csync1", "dummy://foo/bar")
#cs = csync.Create ("/tmp/check_csync1", "dummy://foo/bar")
#cs.set_config_dir ("/tmp/check_csync")

cs = csync.CSync ("testdir", "tmp")
print cs.get_local_only()
cs.set_local_only (True)
print cs.get_local_only()
cs.set_local_only (1)
cs.set_local_only (0)

cs.enable_conflictcopys()
# Note: No function to check status.

exit(0)


## Log level and log callback (module methods)

print csync.get_log_level()
csync.set_log_level(0xff)
loglevel = csync.get_log_level()
assert (loglevel==0xff)
print loglevel

# Q: where is log levels documented?
csync.set_log_level(8)

# Note: If not setting log_callback csync's default log handler is used.
# It outputs messages like this:
# [2013/11/23 18:16:59.370321, 7] csync_lock:  Creating lock file: /home/torkel/.csync/lock

# (int level/verbosity, function_name, log_message)
# Note: log_message is prefixed with function_name (why??)
def log_callback (*args):
    level = str(args[0])
    function = args[1]
    message = args[2][len(function)+2:]
    print "L=" + level + "\t" + message

#csync.set_log_callback (lambda *args: sys.stdout.write(str(args)+"\n"))

csync.set_log_callback (log_callback)
# @todo csync.set_log_callback (None)

assert (csync.get_log_callback() == log_callback)

cs = csync.CSync ("testdir", "tmp")
cs.init()
#cs.update()
#cs.propagate()
#cs.commit()




## Walk local/remote file tree

# (path, modtime, uid, gid, mode, type, instruction)
# @todo document type.
# @todo better to call with dict than tuple?
# @todo expose csync instruction enum
import pwd
from datetime import datetime
def treewalk_visitor (*args):
    #print map(type, args)
    filename = args[0]
    #time = time.ctime (args[1])
    modtime = datetime.fromtimestamp(args[1]).strftime("%F %H:%M")
    user = pwd.getpwuid (args[2])[0]    # slow!
    mode = oct(args[4] & 0b111111111)
    print mode + "\t" + user + "\t" + modtime + "   " + filename


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
