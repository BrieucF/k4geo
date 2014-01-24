#!/usr/bin/python
import MySQLdb
import sys
import time

#--- Mokka DB parameters ---------------
host="pollin1.in2p3.fr"  # your host, usually localhost
user="consult"           # your username
passwd="consult"         # your password
dbName="models03"            # name of the data base
#---------------------------------------

if len( sys.argv ) != 2:
    print " usage: python dumpModelParameters.py MODEL_NAME "
    sys.exit(0)

model   = sys.argv[1]

#model="ILD_o1_v05"

#-----------------------------------------------------------------------
db = MySQLdb.connect(host, user, passwd, dbName ) 
#-----------------------------------------------------------------------

outfile = "model_parameters_"+model+".xml"

file = open( outfile , 'w' )

#-----------------------------------------------------------------------

# create a Cursor object
cur = db.cursor()

# --- select all global parameter for the model 
cur.execute("select sub_detector.driver, sharing.parameter, sharing.driver_default_value from ingredients,sub_detector,sharing where (ingredients.model=\""+model+"\" and ingredients.sub_detector=sub_detector.name and sharing.driver=sub_detector.driver) ;")
#cur.execute("select sub_detector.driver, sharing.parameter, sharing.driver_default_value from ingredients,sub_detector,sharing where (ingredients.model=\""+model+"\" and ingredients.sub_detector=sub_detector.name and sharing.driver=sub_detector.driver and sharing.driver_default_value IS NOT NULL) ;")


#--- param dict:
params = {} 

# --- safe params in dict
for row in cur.fetchall() :
    #print row[1], "  " , row[2]
    params[ row[1] ] = row[2]


#----- now select all model specific parameters
cur.execute("select * from model_parameters where model=\""+model+"\" ;")


#--- overwrite values in the dict:
for row in cur.fetchall() :
    #print row[1], "  " , row[2]
    params[ row[1] ] = row[2]


#dump params to xml file

print >>file, "<!-- "
print >>file, "  global model parameters for model: " + model
print >>file,  "    extracted from Mokka DB at " + host + " - db: " + dbName 
print >>file,  "    on " + time.strftime("%d/%m/%Y")
print >>file,  "    at " + time.strftime("%H:%M:%S")

print >>file, " -->"


for k in sorted( params ):
   v = params[ k ]
   if v:
      print >>file, "<constant name=\"" + k + "\" value=\"" + str(v) + "\"/>"
   else:
      cur.execute("select name, default_value from parameters where name=\"" + k + "\";")
      for row in cur.fetchall() :
          v = row[1]
      print >>file, "<constant name=\"" + k + "\" value=\"" + str(v) + "\"/>"

