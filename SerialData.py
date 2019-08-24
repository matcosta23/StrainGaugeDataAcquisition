import serial    #Permite obter a leitura da porta serial
import time
import pandas as pd 
import matplotlib.pyplot as plt  

ser = serial.Serial('COM3', baudrate = 9600, timeout = 1)       #Leitura da porta serial (deve ser adaptada ao USB utilizado)

LimitTime = float(input("Digite a duração em segundos da execução do ensaio.\n---> "))

tara = 0
tm = 0

UserFile = input("Escreva o nome do arquivo destinado a salvar os dados.\n---> ")

dataFileTXT = open(UserFile + '.txt','w')           #Arquivo.txt
dataFileCSV = open(UserFile + '.csv','w')           #Arquivo.csv

dataFileTXT.write("Deformação\tTempo\n")            #Cabeçalho do arquivo.txt
dataFileTXT.write("um/m\t\ts\n")

dataFileCSV.write("Deformacao,Tempo\n")               #Cabeçalho do arquivo.csv

def GetValue():
    StrainGaugeData = ser.readline().decode().split('\r\n')  #A quebra de linha também é lida na porta serial
    return StrainGaugeData[0]        #Retorna apenas a informação numérica: dado ou tempo

print("\nRespostas positivas devem ser respondidas com 'y' e negativas com 'n'.\n")

userRequest = input("Gostaria de zerar o medidor antes de iniciar?.\n---> ")
if userRequest == 'y':
    tara = 1                        #Marca o pedido do usuário
    
userRequest = input("Para iniciar a captura dos dados, pressione a tecla 'y'.\n---> ")
if userRequest == 'y':
    if tara == 1:                   #Se o usuário solicitou a tara, o comando 't' é escrito no Monitor Serial
        ser.write(b't')
        tara = 0
        
    ser.write(b'g')                 #Comando 'g' inicia a captura de dados e a contagem.
    time.sleep(0.5)                 #Faz o software aguardar meio segundo para não ler a porta serial antes do Arduino processar o primeiro dado
    
    while (tm < (LimitTime)):       #Enquanto o limite solicitado pelo usuário não for atingido.
        
        data = GetValue()           #Primeiro lê a deformação
        dataFileTXT.write(data + '\t\t')
        dataFileCSV(data + ',')     #E escreve de maneira adequada para cada um dos arquivos
        
        tm = float(GetValue())/1000     #Como o tempo lido é em milisegundos, deve ser passado para segundos
        dataFileTXT.write(str(tm) + '\n')
        dataFileTXT.write(str(tm) + '\n')
        
    dataFileTXT.close()
    dataFileCSV.close()             #Fechar os arquivos
    ser.write(b'h')                 #Encerrar o ilustração dos dados na porta serial.
        
#Leitura do arquivo csv para plotar o gráfico.
df = pd.read_csv(UserFile + '.csv')

plt.plot(df['Tempo'],df['Deformacao'],color='r')
plt.xlabel('Tempo (s)')
plt.ylabel('Deformação (um/m)')
plt.title('Deformação obtida no ensaio teste.')

#plt.savefig('graph.png')          #Permite salvar o gráfico
plt.show()
