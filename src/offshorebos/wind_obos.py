from ctypes import *
import csv

# http://stackoverflow.com/a/505457 with GB modifications
class Enum(object):
    def __init__(self, names, separator=None):
        if isinstance(names, type('')):
            names = names.upper().split(separator)
            self.data  = tuple(enumerate(names))
        elif isinstance(names, list):
            for i in names:
                assert len(i) == 2, 'Incorrect input data'
                assert isinstance(i[0], int), 'First index must be an integer'
                assert isinstance(i[1], type('')) or isinstance(i[1], type(u'')), 'Second index must be a string'
            self.data = names
        for i in self.data:
            setattr(self, i[1], i[0]) # name, value
        #for value, name in enumerate(names):
        #    setattr(self, name, value)
    def tuples(self):
        return self.data
    def __getitem__(self, i):
        if isinstance(i, type('')) or isinstance(i, type(u'')):
            return getattr(self, i)
        else:
            return self.data[i][1]
    def __len__(self):
        return len(self.data)
    

# Enumerated types from lib_wind_obos
Substructure    = Enum('MONOPILE JACKET SPAR SEMISUBMERSIBLE')
Anchor          = Enum('DRAGEMBEDMENT SUCTIONPILE')
TurbineInstall  = Enum('INDIVIDUAL BUNNYEARS ROTORASSEMBLED')
TowerInstall    = Enum('ONEPIECE TWOPIECE')
InstallStrategy = Enum('PRIMARYVESSEL FEEDERBARGE')


# Set dynamic library name
from sys import platform
if platform == 'linux' or platform == 'linux2':
    flib = 'lib_wind_obos.so'
elif platform == 'darwin':
    flib = 'lib_wind_obos.dylib'
elif platform == 'win32':
    flib = 'lib_wind_obos.dll'


# Actual wobos class
class wobos(object):
    # Load the library as a static class variable
    cpplib = cdll.LoadLibrary(flib)

    # Establish interface types
    cpplib.pywobos_new.argtypes = []
    cpplib.pywobos_new.restype = c_void_p
    
    cpplib.pywobos_run.argtypes = [c_void_p]
    cpplib.pywobos_run.restype = None
    
    cpplib.pywobos_set_vessel_defaults.argtypes = [c_void_p]
    cpplib.pywobos_set_vessel_defaults.restype = None
    
    cpplib.pywobos_map2variables.argtypes = [c_void_p]
    cpplib.pywobos_map2variables.restype = None
    
    cpplib.pywobos_variables2map.argtypes = [c_void_p]
    cpplib.pywobos_variables2map.restype = None
    
    cpplib.pywobos_set_map_variable.argtypes = [c_void_p, c_char_p, c_double]
    cpplib.pywobos_set_map_variable.restype = None
    
    cpplib.pywobos_get_map_variable.argtypes = [c_void_p, c_char_p]
    cpplib.pywobos_get_map_variable.restype = c_double
    
    def __init__(self):
        # Local wobos object
        self.obj = wobos.cpplib.pywobos_new()

            
    def run(self):
        # Pull variables from map structure and store in class variables
        wobos.cpplib.pywobos_map2variables(self.obj)

        # Since might have reset the substructure, reset the vessel defaults
        wobos.cpplib.pywobos_set_vessel_defaults(self.obj)

        # Run the BOS model
        wobos.cpplib.pywobos_run(self.obj)

        # Copy outputs to map structure for easier access
        wobos.cpplib.pywobos_variables2map(self.obj)


    def variable_access(self, key, val=None):
        # Generic Getter if val is empty, Setter if val is given
        if val is None:
            return wobos.cpplib.pywobos_get_map_variable(self.obj, key)
        else:
            wobos.cpplib.pywobos_set_map_variable(self.obj, key, val)
            return None

        
    def enum_access(self, key, LocalEnum, val=None):
        # Generic Getter if val is empty, Setter if val is given
        assert isinstance(LocalEnum, Enum)
        if val is None:
            return LocalEnum[int(wobos.cpplib.pywobos_get_map_variable(self.obj, key))]
        else:
            wobos.cpplib.pywobos_set_map_variable(self.obj, key, float(LocalEnum[val]))
            return None

    def bool_access(self, key, val=None):
        if val is None:
            return True if wobos.cpplib.pywobos_get_map_variable(self.obj, key)==1.0 else False
        else:
            bval = 1.0 if val else 0.0
            wobos.cpplib.pywobos_set_map_variable(self.obj, key, bval)
            return None


# Variable defaults
wobos_vars = list(csv.reader(open('wind_obos_defaults.csv')))
poplist = []
for k in xrange(len(wobos_vars)):
    # Flag comment lines, empty lines, or non-python relavant variables for removal
    if wobos_vars[k][0][0] == '#' or wobos_vars[k][0].strip() in ['', 'arrayCables', 'exportCables']:
        poplist.append(k)
        continue
    for n in xrange(len(wobos_vars[k])): wobos_vars[k][n] = wobos_vars[k][n].strip()
    # Convert to booleans first
    if wobos_vars[k][-1].upper() == 'TRUE':
        wobos_vars[k][-1] = True
    elif wobos_vars[k][-1].upper() == 'FALSE':
        wobos_vars[k][-1] = False
    else:
        try:
            # Convert to double
            wobos_vars[k][-1] = float(wobos_vars[k][-1])
        except ValueError:
            pass # Keep as string for enumerated types
# Remove unneeded entries (work backwards so index values are still relevant)
poplist.reverse()
for k in poplist: wobos_vars.pop(k)


# Dynamically add accessors methods (Getter/Setter) to wobos class for every variable in defaults
# For a reason I don't quite understand, this has to be done outside of wobos (not in the __init__)
# and divided between a for loop and a separate function (as opposed to stuffing everything in the loop)
def add_variable_fn(fn_name):
   if fn_name in ['cableOptimizer']:
       def fn(self, val=None): return self.bool_access(fn_name, val)
   elif fn_name == 'substructure':
       def fn(self, val=None): return self.enum_access(fn_name, Substructure, val)
   elif fn_name == 'anchor':
       def fn(self, val=None): return self.enum_access(fn_name, Anchor, val)
   elif fn_name == 'turbInstallMethod':
       def fn(self, val=None): return self.enum_access(fn_name, TurbineInstall, val)
   elif fn_name == 'towerInstallMethod':
       def fn(self, val=None): return self.enum_access(fn_name, TowerInstall, val)
   elif fn_name == 'installStrategy':
       def fn(self, val=None): return self.enum_access(fn_name, InstallStrategy, val)
   else:
       def fn(self, val=None): return self.variable_access(fn_name, val)
   
   setattr(wobos, fn_name, fn)

for k in xrange(len(wobos_vars)):
    add_variable_fn(wobos_vars[k][1])
