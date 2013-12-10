''' Log level and log callback (module methods)

Note: If not setting log_callback csync's default log handler is used.
It outputs messages like this:
[2013/11/23 18:16:59.370321, 7] csync_lock:  Creating lock file: /home/torkel/.csync/lock

Q: Where is libcsync's log levels documented?

TODO:
csync.set_log_callback (None)
'''

import sys
import csync


## Log level
print 'Default log level:', csync.get_log_level()

csync.set_log_level(0xff)
loglevel = csync.get_log_level()
assert (loglevel==0xff)

# very verbose logging
csync.set_log_level(8)


## Log callback

#csync.set_log_callback (lambda *args: sys.stdout.write(str(args)+"\n"))

# (int level/verbosity, function_name, log_message)
# Note: log_message is prefixed with function_name
def log_callback (*args):
    level = str(args[0])
    function = args[1]
    message = args[2][len(function)+2:]
    print "L=" + level + "\t" + message

csync.set_log_callback (log_callback)
assert (csync.get_log_callback() == log_callback)


## Now make csync do some logging.
if len(sys.argv) != 3:
    raise RuntimeError ('Need 2 directories to sync')

cs = csync.Create (sys.argv[1], sys.argv[2])
cs.init()
cs.update()
#cs.reconcile()
#cs.propagate()
#cs.commit()
