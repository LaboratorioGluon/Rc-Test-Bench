import serial


# Configuration
NUM_MEDIDAS = 10

def extractDataFromLine(line):
    return int(line.replace(b"Data: ", b""))


def mean(data_array):
    acum = 0
    for d in data_array:
        acum = acum + d

    return acum/len(data_array)

ser = serial.Serial('COM35', 57600)



while input("'q' para salir:") != 'q':
    
    ser.flush()
    ser.flushOutput()
    ser.flushInput()

    print("Calibrando ...")
    datos = []
    
    for i in range(NUM_MEDIDAS):
        datos.append( extractDataFromLine(ser.readline()) )
    
    print("== Resultados ==")
    print("Max: " + str(max(datos)))
    print("Min: " + str(min(datos)))
    print("Media: " + str(mean(datos)))

ser.close()
