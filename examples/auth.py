''' Auth callback '''

import sys
import getpass
import csync

#cs = csync.Create ("/tmp/test", "sftp://srv1/home/torkel/foobar")

if len(sys.argv) != 3:
    raise RuntimeError ('usage: auth.py /path/to/dir sftp://example.org/path')

cs = csync.Create (sys.argv[1], sys.argv[2])

# NOTE: No way to break out / abort password reading. This is IMHO a
# limitation of libcsync.
# @todo what is echo & verify for?
def auth_callback (prompt, echo, verify):
    return getpass.getpass (prompt)
    # Note: getpass() raises EOFError on ctrl-d

cs.set_auth_callback (auth_callback)
#cs.set_auth_callback (lambda prompt,*ignore: getpass.getpass(prompt))

cs.init()
