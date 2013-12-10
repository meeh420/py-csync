import sys
sys.path.insert (0, 'build/lib.linux-x86_64-2.7/') # @todo don't hardcode
import csync

print '=== BEGIN TEST ==='

#cs = csync.CSync ("testdir", "sftp://srv1/home/torkel/foobar")
#cs = csync.CSync ("/tmp/check_csync1", "dummy://foo/bar")
#cs = csync.Create ("/tmp/check_csync1", "dummy://foo/bar")
#cs.set_config_dir ("/tmp/check_csync")

# $ csync testdir sftp://srv1/home/torkel/foobar
# ERROR:
# csync_timediff: Unable to create temporary file: sftp://srv1/home/torkel/foobar/.csync_timediff.ctmp - 
# csync_init:  Clock skew detected. The time difference is greater than 10 seconds!
# csync_init: No such file or directory

csync.set_log_callback (lambda *args: sys.stdout.write(str(args)+"\n"))

def file_progress_cb (*args):
    print args[0]
    #print args
    #print map(type, args)

# XXX callbacks not called! is it becaus doing local only sync?
#cs = csync.CSync ("testdir", "tmp")
cs = csync.CSync ("testdir", "sftp://srv1/home/torkel/foobar")
import getpass
cs.set_auth_callback (lambda prompt,u1,u2: getpass.getpass(prompt))
cs.set_file_progress_callback (file_progress_cb)
cs.set_overall_progress_callback (file_progress_cb)
cs.init()
cs.update()
cs.propagate()
#cs.reconcile()
#cs.commit()

exit(0)


#def auth_callback (*args):
#    print map(type, args)  # @todo bool not int?
#    print '\tnum_args = ' + len(args)
#    return 'password'
#    return None     # error!


"""
cs = csync.CSync ("testdir", "tmp")
print cs.get_status_string()
# better to return empty string instead of None?
#print "Status string: " + str(cs.get_status_string())
try:
    cs.set_iconv_codec ("latin1")   # iconv source codec for filenames
except AttributeError:
    print "NOTE: CSync compiled without iconv support."
"""




"""
cs = csync.CSync ("testdir", "tmp")
print cs.get_local_only()
cs.set_local_only (True)
print cs.get_local_only()
cs.set_local_only (1)
cs.set_local_only (0)

cs.enable_conflictcopys()
# Note: No function to check status.
"""









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
