
from building import *

cwd     = GetCurrentDir()
path    = [cwd]
src     = Glob('*.c') + Glob('*.cpp')

group = DefineGroup('vector', src, depend = ['PKG_USING_VECTOR'], CPPPATH = path)
Return('group')
