import csync

try:
    cs = csync.Create ("/nonexistent1", "/nonexistent2")
    cs.init()
except csync.Error as e:
    print 'Got exception of type', type(e)
    print 'Members:', [s for s in dir(e) if not s.startswith('__')]
    print 'Message:', e.message
    print 'Status: ', e.status
    assert (e.status == csync.Status.STATEDB_LOAD_ERROR)


print
cs = csync.Create ("/nonexistent1", "/nonexistent2")
cs.init()
