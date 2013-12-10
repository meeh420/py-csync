''' Walk local/remote file tree

'''

import sys
import csync

if len(sys.argv) != 3:
    raise RuntimeError ('Need 2 directories to operate on.')


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


cs = csync.Create (sys.argv[1], sys.argv[2])
cs.init()
cs.update()
# NOTE walk_*_tree must be called after 'update' and before 'propagate'
cs.walk_local_tree (treewalk_visitor, 0xff)
#cs.walk_remote_tree (treewalk_visitor, 0xff)
