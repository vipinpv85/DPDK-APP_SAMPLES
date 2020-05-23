import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import csv
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--file',  required=True, help='csv file to plot')
#args = parser.parse_args()
args = vars(parser.parse_args())

#parser.print_help()
#print("Hi there {}, it's nice to meet you!".format(args["file"]))

starttime = 0
maxactual = 0
time = []
request = []
actual = []
diff = []

csvFile = csv.reader(open(args["file"]), delimiter=',', quotechar='|')
csvFile.next()


for row in csvFile:
  #print " r  1 " + str(row[2]) + " row 2 " + str(row[3])

  if (len(time) == 0):
    starttime = int(row[0])
    time.append(0)
  else:
    time.append(int(row[0]) - starttime)

  request.append(row[2])
  actual.append(row[3])
  diff.append(int(row[2]) - int(row[3]))

  if (maxactual < int(row[3])):
    maxactual = int(row[3])

  if (csvFile.line_num > 250):
    break

#plt.axis([0,int(time[-1]), 0, 2 * maxactual])
#plt.axis()

#width = 0.35
#p1 = plt.bar(time, request, width) 
#p2 = plt.bar(time, actual, width, bottom=diff)
#
#plt.title('Eventdev Deq')
#plt.xlabel('time slot diff')
#plt.ylabel('number of events')
#plt.legend((p1[0], p2[0]), ('Request', 'Actual'))
#
#plt.plot(time, request, 'ro', time, actual, 'gs')
#
#plt.show()

#lines = plt.plot(time, request, time, actual)
#plt.setp(lines, color='r', linewidth=2.0)

fig = plt.figure()

fig.add_subplot(3,2,1)
plt.plot(time, request, 'ro')
plt.title("Deq request")

fig.add_subplot(3,2,2)
plt.plot(time, actual, 'bo')
plt.title("Deq actual")

fig.add_subplot(3,1,3)
plt.scatter(time, diff)
plt.title("Deq request-actual")

plt.suptitle('Evenetdev Dequeue for ' + str(args["file"].split("-")[0]))

plt.show()
