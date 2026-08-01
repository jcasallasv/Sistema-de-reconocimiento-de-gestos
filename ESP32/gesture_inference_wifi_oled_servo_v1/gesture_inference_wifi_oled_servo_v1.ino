
Today 12:48 AM

Pasted text(10).txt
Document

Worked for 42s
Recibido. Continuamos exactamente desde el estado estable: CNN real desplegada, las tres clases validadas en vivo, rechazo de captura estacionaria funcionando y ningún cambio de firmware antes del video de respaldo. Por una vez, la estrategia correcta consiste en no tocar lo que ya funciona. Milagros de la ingeniería. 


Etapa 1: preparar todo para grabar
1. Red y alimentación
Conecta el PC a la corriente.

En el iPhone, activa:

Personal Hotspot

Maximize Compatibility

Confirma que el PC esté conectado al hotspot.

Energiza la Raspberry Pi y espera aproximadamente 90 segundos.

Conecta el ESP32 al PC por USB.

No muevas todavía ningún jumper ni componente.

2. Verificar la Raspberry Pi
En Windows PowerShell, ejecuta:

ssh salo@172.20.10.7 "systemctl is-active gesture-server.service; hostname -I"
Resultado esperado:

active
172.20.10.7
Después, todavía en Windows PowerShell, ejecuta:

Invoke-RestMethod -Uri "http://172.20.10.7:5000/api/status" | ConvertTo-Json -Depth 5
Debe aparecer una respuesta JSON sin errores. No ejecutes python app.py; el servidor ya está controlado por systemd.

3. Preparar Arduino IDE
Abre Arduino IDE 2.3.6.

Abre este sketch, sin modificarlo:

C:\Users\luisfer\iCloudDrive\U\SISTEMAS EMBEBIDOS\PROYECTO\ESP32\gesture_inference_wifi_apds9960\gesture_inference_wifi_apds9960.ino
Confirma:

Board: DOIT ESP32 DEVKIT V1

Port: COM3

Abre Serial Monitor.

Configura:

115200 baud
Presiona una sola vez el botón EN del ESP32.

Espera hasta que aparezca la conexión WiFi y el sistema quede listo para capturar.

No recompiles ni cargues el firmware. La idea del respaldo es grabar la versión que ya sobrevivió a nuestras decisiones humanas.

4. Preparar el montaje físico
Acomoda el prototipo para que sean visibles:

ESP32.

MPU6050.

Push button.

Capture LED.

Mano realizando el gesto.

Cable USB, sin tensión mecánica sobre la protoboard.

Deja suficiente espacio para realizar claramente:

left

right

up

Mantén el sensor con la misma orientación utilizada durante la recolección del dataset. No gires la protoboard respecto a la posición de las pruebas exitosas.

5. Preparar la cámara
Configura el teléfono así:

Video horizontal.

Resolución 1080p.

Al menos 2 GB de espacio libre.

Batería superior al 30% o conectado a alimentación.

Lente limpia.

Sin música ni ruido innecesario.

El hotspot debe continuar activo mientras se abre la cámara.

Ubica la cámara de forma que el encuadre incluya simultáneamente:

El hardware.

El LED.

Tu mano.

El Serial Monitor, suficientemente cerca para leer el resultado.

Todavía no grabes los gestos.

Cuando termines esta preparación, envía una captura donde se vean PowerShell con active y el Arduino Serial Monitor listo. Con eso hacemos la grabación en una secuencia única y controlada.



Recuerda que las carpetas que hay dentro de PROYECTO son estas. Tú mismo me dijiste que creara esas carpetas con esos mismos nombres cuando empezamos con este proyecto. Acaso ya has borrado información de tus memorias?

Listo. La Raspberry está otra vez en 172.20.10.7, con SSH y Flask activos. No más barridos de once minutos, esa fue una idea espantosa y no la repetiremos.

Vamos a dejarle IP fija 172.20.10.7, autoconexión y WiFi power saving desactivado. Así el ESP32 puede seguir usando la dirección que ya tiene configurada.

En Windows PowerShell, ejecuta este bloque completo:

$remoteScript = @'
set -e

PROJECT_PATH="/home/salo/embedded_gesture_system"
BACKUP_PATH="$PROJECT_PATH/backups/network_static_ip_v1"
CONNECTION_NAME="$(nmcli -g GENERAL.CONNECTION device show wlan0)"

if [ -z "$CONNECTION_NAME" ] || [ "$CONNECTION_NAME" = "--" ]; then
    echo "ERROR: No active WiFi connection was found on wlan0."
    exit 1
fi

mkdir -p "$BACKUP_PATH"

nmcli connection show "$CONNECTION_NAME" > "$BACKUP_PATH/connection_before.txt"
ip -4 address show wlan0 > "$BACKUP_PATH/ip_before.txt"

sudo mkdir -p /etc/NetworkManager/conf.d

printf '[connection]\nwifi.powersave = 2\n' |
    sudo tee /etc/NetworkManager/conf.d/99-wifi-powersave-off.conf > /dev/null

sudo nmcli connection modify "$CONNECTION_NAME" \
    ipv4.method manual \
    ipv4.addresses "172.20.10.7/28" \
    ipv4.gateway "172.20.10.1" \
    ipv4.dns "172.20.10.1 1.1.1.1" \
    connection.autoconnect yes \
    connection.autoconnect-priority 100

echo "ACTIVE_CONNECTION=$CONNECTION_NAME"
echo "STATIC_CONFIGURATION_SAVED=True"
echo "The SSH connection may close while WiFi is reapplied."

sudo nmcli connection up "$CONNECTION_NAME"
'@

ssh salo@172.20.10.7 $remoteScript
Puede pedir la contraseña de sudo. Al final, SSH puede cerrarse o mostrar Connection reset; eso es normal, porque estamos reconfigurando precisamente la conexión que estamos usando. Ingeniería remota, el arte de serruchar cuidadosamente la rama sin caerse.

Espera 20 segundos y ejecuta:

Test-NetConnection 172.20.10.7 -Port 22
Debe aparecer:

TcpTestSucceeded : True
Luego verifica toda la configuración:

ssh salo@172.20.10.7 'CONNECTION_NAME="$(nmcli -g GENERAL.CONNECTION device show wlan0)"; echo "ACTIVE_CONNECTION=$CONNECTION_NAME"; echo "IP_CONFIGURATION:"; nmcli -g ipv4.method,ipv4.addresses,ipv4.gateway,connection.autoconnect connection show "$CONNECTION_NAME"; echo "WIFI_POWER_SAVE:"; iw dev wlan0 get power_save; echo "GESTURE_SERVER:"; systemctl is-enabled gesture-server.service; systemctl is-active gesture-server.service'
El resultado correcto debe contener:

manual
172.20.10.7/28
172.20.10.1
yes
Power save: off
enabled
active
Con eso queda fija la Raspberry en .7 y retomamos inmediatamente el dashboard.



app_before_dashboard_v1.py
Python
Hago un paréntesis para afanarte. Presentamos esto en 7 horas y no lo tenemos listo.

image(92).png


gesture_inference_wifi_apds9960(4).ino
File


DOCUMENTO.tex
File
Te paso la plantilla que usaremos para el proyecto. Solo necesito que hagas un borrador, no agregues portada, ya que con el input de PORTADA, se llama el archivo de portada que tengo yo, el cual lleva la info propia del proyecto. Solo agrega las secciones siguiendo el formato en el que están las secciones, subsecciones y subsubsecciones.
Usa secciones, subsecciones, subsubsecciones y parágrafos cuando sea necesario.
Al final del documento hay una referencia summy que sirve para que veas le formato en que deben ir las referencias.
Para las tablas, asegúrate de que el caption quede debajo y que no diga "Cuadro 1", sino "Tabla 1".

Worked for 6m 9s
Listo. Preparé un borrador completo en LaTeX sin agregar una portada nueva y conservando:

\input{PORTADA}
El documento incluye secciones, subsecciones, subsubsecciones y párrafos; arquitectura con diagrama TikZ; hardware; firmware; OLED dinámica; servo; dataset; CNN; Raspberry Pi; dashboard; resultados; limitaciones; conclusiones y referencias con el formato de la plantilla.

Las tablas tienen el caption debajo y están configuradas para mostrarse como:

Tabla 1
También validé la sintaxis compilando una copia con recursos provisionales: genera correctamente un documento de 15 páginas. Naturalmente, para compilar el archivo real deben permanecer disponibles PORTADA.tex, las carpetas de fuentes Ancízar y las imágenes originales de la plantilla. LaTeX exige sus rituales completos o se ofende.

Descargar DOCUMENTO_BORRADOR.tex


Damelo en español.
No me des el archivo sino el código para pegar en texstudio. Si me das archivos, tengo que descargarlos, cambiarlos de ubicación y luego cambiarles el nombre y claramente no quiero eso.

Worked for 1m 20s
Tienes razón. Nada de archivos para que Windows convierta una tarea sencilla en una mudanza internacional. El borrador queda completamente en español y conserva la estructura de la plantilla, el llamado a PORTADA, las jerarquías de títulos y las referencias tipo \bibitem. El contenido técnico se basa en el estado real del proyecto, incluidos el modelo, las métricas, la arquitectura y el servidor Flask. 

 

 


Copia y reemplaza el contenido de DOCUMENTO.tex por lo siguiente. Compílalo con XeLaTeX, no con pdfLaTeX, porque la plantilla usa fontspec.

\documentclass[12pt,hyphens]{article}

\usepackage{fontspec}
\usepackage{tikz}
\usetikzlibrary{positioning,arrows.meta,shapes.geometric}
\usepackage{circuitikz}
\usepackage{amsmath}
\usepackage{hyperref}
\usepackage{comment}
\usepackage{setspace}
\usepackage{fancyhdr}
\usepackage{titlesec}
\usepackage{ifthen}
\usepackage{tabularx}
\usepackage{ragged2e}
\usepackage{multirow}
\usepackage{float}
\usepackage{booktabs}
\usepackage{caption}
\usepackage[table,xcdraw]{xcolor}
\usepackage[export]{adjustbox}
\usepackage{url}
\usepackage{geometry}
\usepackage{polyglossia}

\newcolumntype{L}{>{\RaggedRight\hangafter=1\hangindent=1.5em}X}
\newcommand\vn[1]{\textit{#1}}

\geometry{
	top=2.5cm,
	inner=2.5cm,
	outer=2.5cm,
	bottom=2.5cm,
	headheight=0.7ex,
	headsep=11ex,
}

\setdefaultlanguage{spanish}
\setcounter{secnumdepth}{4}

\newfontfamily\AncizarSans[
Extension={.otf}, Path=./AncizarSans/,
UprightFont={*-Regular}, BoldFont={*-Bold},
ItalicFont={*-Italic}, BoldItalicFont={*-BoldItalic}
]{AncizarSans}

\newfontfamily\AncizarSerif[
Extension={.otf}, Path=./AncizarSerif/,
UprightFont={*-Regular}, BoldFont={*-Bold},
ItalicFont={*-Italic}, BoldItalicFont={*-BoldItalic}
]{AncizarSerif}

\newcommand{\fontoption}{sans}

\ifthenelse{\equal{\fontoption}{sans}}{
	\setmainfont{AncizarSans}[
	Extension={.otf}, Path=./AncizarSans/,
	UprightFont={*-Regular}, BoldFont={*-Bold},
	ItalicFont={*-Italic}, BoldItalicFont={*-BoldItalic}]
}{
	\setmainfont{AncizarSerif}[
	Extension={.otf}, Path=./AncizarSerif/,
	UprightFont={*-Regular}, BoldFont={*-Bold},
	ItalicFont={*-Italic}, BoldItalicFont={*-BoldItalic}]
}

\newcommand{\textsans}[1]{{\AncizarSans #1}}
\newcommand{\textserif}[1]{{\AncizarSerif #1}}
\newenvironment{SansBlock}{\begingroup\AncizarSans}{\endgroup}
\newenvironment{SerifBlock}{\begingroup\AncizarSerif}{\endgroup}

\begin{document}

% La portada se encuentra en un archivo independiente.
\input{PORTADA}

% Se obliga a LaTeX a utilizar la palabra "Tabla" en lugar de "Cuadro".
\renewcommand{\tablename}{Tabla}

% Los títulos de las tablas se ubican debajo de estas.
\captionsetup[table]{
	position=bottom,
	skip=8pt
}

\thispagestyle{plain}
\tableofcontents
\newpage

\pagestyle{fancy}
\fancyhead{}
\fancyfoot{}

\fancyhead[L]{\textit{Universidad Nacional de Colombia}}

\fancyhead[C]{%
	\ifnum\value{page}=3
		\includegraphics[
			width=0.06\linewidth
		]{"IMAGENES/FreeSample-Vectorizer-io-unnamed"}
	\else
		\includegraphics[
			width=0.06\linewidth
		]{"IMAGENES/ESCUDO_NEGRO"}
	\fi
}

\fancyhead[R]{%
	\textit{Depto. de Ingeniería Eléctrica y Electrónica}
}

\fancyfoot[C]{\thepage}

\singlespacing


% =========================================================
% RESUMEN
% =========================================================

\section{\textit{Resumen}}

En este documento se presenta el diseño y la implementación de un sistema
embebido inalámbrico para el reconocimiento de gestos. El prototipo integra
un microcontrolador ESP32, un sensor inercial MPU6050, una Raspberry Pi Zero
W, una red neuronal convolucional unidimensional, una interfaz web, una
pantalla OLED y actuadores físicos.

El proceso comienza mediante un pulsador conectado a una interrupción del
ESP32. Después de la activación, el microcontrolador adquiere durante
aproximadamente dos segundos un total de 100 muestras provenientes del
acelerómetro y del giróscopo del MPU6050. Cada muestra contiene seis canales:
aceleración en los ejes $x$, $y$ y $z$, y velocidad angular en los mismos
tres ejes.

Las muestras se transmiten mediante WiFi, utilizando HTTP y una estructura
JSON, hacia un servidor Flask ejecutado en una Raspberry Pi Zero W. La
Raspberry procesa la secuencia mediante una red neuronal convolucional 1D
implementada con operaciones de NumPy y retorna una de tres clases:
\textit{left}, \textit{right} o \textit{up}, junto con la confianza, las
probabilidades de cada clase y el tiempo de inferencia.

Para el entrenamiento se construyó un conjunto de datos balanceado con 120
capturas reales, correspondientes a 40 capturas por clase. Durante la
recolección, el sensor APDS-9960 se utilizó como referencia para asignar la
etiqueta del gesto. Sin embargo, durante la inferencia final únicamente se
utilizan las señales provenientes del MPU6050.

El modelo obtuvo una exactitud de prueba del 83,33\,\%. El sistema también
incluye rechazo de capturas estacionarias, una interfaz OLED dinámica basada
en estados, un servomotor asociado a la dirección reconocida, un LED de
señalización, un panel web y el inicio automático del servidor mediante
\texttt{systemd}.

\paragraph{\textit{Palabras clave.}}
Sistemas embebidos, ESP32, Raspberry Pi Zero W, MPU6050, reconocimiento de
gestos, CNN 1D, NumPy, Flask, IoT, OLED y servomotor.


% =========================================================
% INTRODUCCIÓN
% =========================================================

\section{\textit{Introducción}}

\subsection{\textit{Contexto}}

Un sistema embebido integra recursos de procesamiento, memoria,
comunicaciones, sensores, actuadores, alimentación y software para realizar
una función específica. En el prototipo desarrollado, el ESP32 se encarga de
la adquisición de señales, la interacción con los sensores, la comunicación
inalámbrica y el control de los actuadores. Por otro lado, la Raspberry Pi
Zero W proporciona un entorno Linux embebido en el cual se ejecutan el
servidor web y el modelo de clasificación \cite{gallego_componentes}.

Esta división permite aprovechar las características de cada plataforma. El
microcontrolador realiza las tareas relacionadas con el hardware y los
tiempos de captura, mientras que la Raspberry Pi ejecuta tareas de software
de mayor nivel, como la recepción de solicitudes HTTP, el procesamiento
numérico y la visualización mediante una interfaz web.

El reconocimiento de gestos constituye un problema apropiado para integrar
sensores, comunicaciones inalámbricas, procesamiento de secuencias y
aprendizaje automático. A diferencia de un clasificador basado únicamente en
umbrales escritos manualmente, una red neuronal aprende patrones a partir de
ejemplos etiquetados \cite{gallego_ia}.

\subsection{\textit{Planteamiento del problema}}

Se requiere desarrollar un sistema capaz de reconocer movimientos
realizados por una persona utilizando un sensor inercial de bajo costo. El
sistema debe adquirir de manera consistente las señales del movimiento,
transmitirlas inalámbricamente, ejecutar un modelo de clasificación en una
plataforma embebida y generar una respuesta visible y mecánica.

Los principales retos técnicos del proyecto son los siguientes:

\begin{itemize}
	\item Mantener un intervalo de muestreo cercano a 20 ms.
	\item Diferenciar los gestos \textit{left}, \textit{right} y
	\textit{up}.
	\item Evitar que una captura sin movimiento sea clasificada como un
	gesto válido.
	\item Ejecutar la inferencia en una Raspberry Pi Zero W con recursos
	limitados.
	\item Mantener estable la comunicación WiFi entre el ESP32 y la
	Raspberry.
	\item Integrar interfaces y actuadores sin alterar la captura del
	MPU6050.
\end{itemize}

\subsection{\textit{Objetivos}}

\subsubsection{\textit{Objetivo general}}

Diseñar e implementar un sistema embebido inalámbrico capaz de clasificar
gestos a partir de las señales inerciales de un MPU6050, integrando un ESP32,
una Raspberry Pi Zero W, una red neuronal convolucional 1D, una interfaz IoT
y actuadores físicos.

\subsubsection{\textit{Objetivos específicos}}

\begin{itemize}
	\item Adquirir 100 muestras del MPU6050 a una frecuencia aproximada de
	50 Hz.
	\item Iniciar el proceso de captura mediante un pulsador conectado a una
	interrupción.
	\item Construir un conjunto de datos balanceado con gestos reales.
	\item Entrenar y evaluar una red neuronal convolucional unidimensional.
	\item Implementar el modelo entrenado en la Raspberry Pi mediante
	operaciones de NumPy.
	\item Transmitir los datos mediante WiFi utilizando HTTP y JSON.
	\item Mostrar dinámicamente los estados del sistema en una pantalla
	OLED.
	\item Asociar cada gesto reconocido con una posición del servomotor.
	\item Visualizar el resultado de la inferencia en una interfaz web.
	\item Rechazar capturas en las cuales no se detecte movimiento
	suficiente.
\end{itemize}


% =========================================================
% ALCANCE Y REQUISITOS
% =========================================================

\section{\textit{Alcance y requisitos del sistema}}

\subsection{\textit{Alcance funcional}}

El sistema desarrollado realiza la siguiente secuencia general:

\begin{enumerate}
	\item El usuario presiona el pulsador.
	\item El ESP32 detecta la pulsación mediante una interrupción.
	\item La interfaz OLED indica que se debe preparar el gesto.
	\item El ESP32 captura 100 muestras del MPU6050.
	\item El firmware valida que haya ocurrido movimiento suficiente.
	\item Las muestras aceptadas se transmiten a la Raspberry Pi.
	\item La Raspberry ejecuta la inferencia con la CNN 1D.
	\item El servidor retorna la clase, la confianza y las probabilidades.
	\item El ESP32 actualiza la pantalla OLED.
	\item El servomotor se mueve a la posición asociada con el gesto.
	\item El LED genera un patrón visual correspondiente al resultado.
	\item El sistema regresa al estado de espera.
\end{enumerate}

\subsection{\textit{Requisitos principales}}

\begin{table}[H]
	\centering
	\begin{tabularx}{\textwidth}{
		p{0.27\textwidth}
		L
	}
		\toprule
		\textbf{Requisito} & \textbf{Implementación} \\
		\midrule

		Microcontrolador &
		ESP32 DevKit V1 encargado de adquisición, comunicación y control. \\

		Plataforma con Linux &
		Raspberry Pi Zero W con Raspberry Pi OS Lite de 32 bits. \\

		Sensor inercial &
		MPU6050 con acelerómetro y giróscopo de tres ejes. \\

		Interrupción &
		Pulsador conectado a GPIO27 con interrupción por flanco descendente. \\

		Comunicación inalámbrica &
		WiFi de 2,4 GHz, solicitudes HTTP y mensajes JSON. \\

		Inteligencia artificial &
		Red neuronal convolucional 1D entrenada con datos reales. \\

		Interfaz local &
		Pantalla OLED SSD1306 de 128 por 64 píxeles mediante SPI. \\

		Interfaz remota &
		Panel web implementado con Flask. \\

		Actuador mecánico &
		Servomotor SG90 asociado a la dirección reconocida. \\

		Actuador visual &
		LED utilizado para indicar captura y resultado. \\

		Confiabilidad &
		Rechazo de capturas estacionarias, dirección IP fija, inicio
		automático y respaldos verificados. \\

		\bottomrule
	\end{tabularx}
	\caption{Requisitos principales del sistema embebido.}
	\label{tab:requisitos}
\end{table}

\subsection{\textit{Restricciones de diseño}}

\paragraph{\textit{Capacidad de procesamiento.}}

La Raspberry Pi Zero W V1.1 utiliza una arquitectura ARMv6 y dispone de una
capacidad de procesamiento considerablemente inferior a la de modelos más
recientes. En el entorno instalado no se encontraba disponible
\texttt{tflite\_runtime}, y la instalación completa de TensorFlow no era
viable. Por esta razón se implementó una inferencia basada exclusivamente en
NumPy.

\paragraph{\textit{Restricciones de red.}}

La Raspberry Pi Zero W únicamente admite redes WiFi de 2,4 GHz. Para las
pruebas y la demostración se empleó el punto de acceso de un teléfono móvil
con la opción de compatibilidad activada. La Raspberry se configuró con la
dirección fija:

\begin{center}
	\texttt{172.20.10.7}
\end{center}

\paragraph{\textit{Restricciones temporales.}}

La actualización de la pantalla OLED y el movimiento del servo no deben
realizarse dentro del ciclo de adquisición de las 100 muestras. Estas
operaciones podrían aumentar el tiempo entre muestras y modificar las
características de la señal entregada al modelo.


% =========================================================
% ARQUITECTURA
% =========================================================

\section{\textit{Arquitectura del sistema}}

\subsection{\textit{Arquitectura general}}

La arquitectura utiliza el ESP32 como cliente de adquisición y control, y la
Raspberry Pi como servidor de inferencia y visualización. La
Figura~\ref{fig:arquitectura} representa los principales bloques del
sistema.

\begin{figure}[H]
	\centering

	\begin{tikzpicture}[
		node distance=1.1cm and 1.35cm,
		bloque/.style={
			draw,
			rounded corners,
			align=center,
			minimum width=3.0cm,
			minimum height=1.1cm,
			fill=gray!8
		},
		flecha/.style={
			-{Latex[length=3mm]},
			thick
		}
	]

		\node[bloque] (boton) {
			Pulsador\\
			Interrupción
		};

		\node[bloque, right=of boton] (esp32) {
			ESP32\\
			Adquisición y control
		};

		\node[bloque, above=of esp32] (mpu) {
			MPU6050\\
			100 $\times$ 6 muestras
		};

		\node[bloque, below=of esp32] (salidas) {
			OLED, servo y LED\\
			Salidas físicas
		};

		\node[bloque, right=of esp32] (wifi) {
			WiFi\\
			HTTP y JSON
		};

		\node[bloque, right=of wifi] (rpi) {
			Raspberry Pi Zero W\\
			Flask y CNN con NumPy
		};

		\node[bloque, below=of rpi] (dashboard) {
			Panel web\\
			Estado y resultados
		};

		\draw[flecha] (boton) -- (esp32);
		\draw[flecha] (mpu) -- (esp32);
		\draw[flecha] (esp32) -- (wifi);

		\draw[flecha]
		(wifi) --
		node[above]{Captura}
		(rpi);

		\draw[flecha]
		(rpi) --
		node[below]{Predicción}
		(wifi);

		\draw[flecha] (esp32) -- (salidas);
		\draw[flecha] (rpi) -- (dashboard);

	\end{tikzpicture}

	\caption{Arquitectura general del sistema de reconocimiento de gestos.}
	\label{fig:arquitectura}
\end{figure}

\subsection{\textit{Distribución de funciones}}

\subsubsection{\textit{Funciones del ESP32}}

El ESP32 realiza las siguientes tareas:

\begin{itemize}
	\item Inicialización de sensores y actuadores.
	\item Gestión de la interrupción del pulsador.
	\item Adquisición periódica de las señales inerciales.
	\item Cálculo de valores pico a pico.
	\item Rechazo de capturas con movimiento insuficiente.
	\item Construcción del mensaje JSON.
	\item Comunicación HTTP con la Raspberry Pi.
	\item Interpretación de la respuesta del servidor.
	\item Actualización de la pantalla OLED.
	\item Control del servomotor y del LED.
\end{itemize}

\subsubsection{\textit{Funciones de la Raspberry Pi}}

La Raspberry Pi realiza las siguientes tareas:

\begin{itemize}
	\item Ejecución de un servidor Flask.
	\item Validación de las solicitudes recibidas.
	\item Conversión de las muestras a un arreglo NumPy.
	\item Normalización de los seis canales.
	\item Inferencia mediante la CNN 1D.
	\item Generación de la respuesta JSON.
	\item Visualización del último resultado en el panel web.
	\item Inicio automático del servidor después del arranque.
\end{itemize}


% =========================================================
% HARDWARE
% =========================================================

\section{\textit{Implementación de hardware}}

\subsection{\textit{Componentes principales}}

\begin{table}[H]
	\centering
	\begin{tabularx}{\textwidth}{
		p{0.23\textwidth}
		p{0.24\textwidth}
		L
	}
		\toprule
		\textbf{Componente} &
		\textbf{Función} &
		\textbf{Características relevantes} \\
		\midrule

		ESP32 DevKit V1 &
		Cliente MCU &
		Módulo ESP32-WROOM-32 con WiFi, GPIO e interrupciones. \\

		Raspberry Pi Zero W V1.1 &
		Servidor MPU &
		Raspberry Pi OS Lite, arquitectura ARMv6 y servidor Flask. \\

		MPU6050 &
		Sensor de movimiento &
		Acelerómetro y giróscopo de tres ejes; dirección I2C
		\texttt{0x68}. \\

		APDS-9960 &
		Sensor de referencia &
		Utilizado para etiquetar los gestos durante la adquisición del
		conjunto de datos. \\

		OLED SSD1306 &
		Interfaz de estados &
		Pantalla monocromática de 128 por 64 píxeles con interfaz SPI. \\

		Servo SG90 &
		Actuador mecánico &
		Movimiento angular asociado a la clase reconocida. \\

		Pulsador &
		Entrada de usuario &
		Activa la captura mediante una interrupción. \\

		LED &
		Actuador visual &
		Indica captura, resultado y condiciones de error. \\

		\bottomrule
	\end{tabularx}
	\caption{Componentes principales utilizados en el prototipo.}
	\label{tab:componentes}
\end{table}

\subsection{\textit{Asignación de pines del ESP32}}

\begin{table}[H]
	\centering
	\begin{tabularx}{\textwidth}{
		p{0.30\textwidth}
		p{0.19\textwidth}
		L
	}
		\toprule
		\textbf{Señal} &
		\textbf{Pin del ESP32} &
		\textbf{Descripción} \\
		\midrule

		MPU6050 SDA / APDS-9960 SDA &
		GPIO21 &
		Línea de datos del bus I2C compartido. \\

		MPU6050 SCL / APDS-9960 SCL &
		GPIO22 &
		Línea de reloj del bus I2C a 100 kHz. \\

		Pulsador &
		GPIO27 &
		Entrada con \texttt{INPUT\_PULLUP} e interrupción por flanco
		descendente. \\

		LED &
		GPIO26 &
		Indicación visual del proceso y del resultado. \\

		Buzzer activo &
		GPIO25 &
		Salida previamente validada mediante transistor 2N2222A. \\

		OLED MOSI &
		GPIO23 &
		Línea de datos de la pantalla SPI. \\

		OLED CLK &
		GPIO18 &
		Reloj de la pantalla SPI. \\

		OLED DC &
		GPIO2 &
		Selección entre datos y comandos. \\

		OLED CS &
		GPIO5 &
		Selección de la pantalla OLED. \\

		OLED RESET &
		GPIO4 &
		Reinicio de la pantalla. \\

		Señal del servo &
		GPIO13 &
		Señal PWM de 50 Hz para el SG90. \\

		\bottomrule
	\end{tabularx}
	\caption{Asignación de pines para sensores, interfaz y actuadores.}
	\label{tab:pines}
\end{table}

\subsection{\textit{Pantalla OLED como interfaz de estados}}

La pantalla OLED no se utiliza para mostrar un mensaje estático. Se diseñó
como una interfaz dinámica que informa la etapa en la que se encuentra el
sistema.

Los principales estados representados son:

\begin{itemize}
	\item \texttt{BOOTING}.
	\item \texttt{INITIALIZING}.
	\item \texttt{CONNECTING WIFI}.
	\item \texttt{SYSTEM READY}.
	\item \texttt{PREPARE GESTURE}.
	\item \texttt{CAPTURING}.
	\item \texttt{VALIDATING}.
	\item \texttt{SENDING DATA}.
	\item \texttt{CNN PROCESSING}.
	\item \texttt{ACTUATING}.
	\item \texttt{GESTURE RESULT}.
	\item \texttt{MOTION REJECTED}.
	\item \texttt{NETWORK ERROR}.
	\item \texttt{SERVER ERROR}.
\end{itemize}

\paragraph{\textit{Criterio temporal.}}

La función encargada de transferir el contenido del búfer a la pantalla no
se ejecuta dentro del ciclo de adquisición de las 100 muestras. Las
actualizaciones se realizan antes o después de la captura para evitar que la
interfaz modifique el periodo nominal de muestreo.

\subsection{\textit{Mapeo del servomotor}}

El código de validación utilizado para la OLED y el servomotor estableció el
siguiente mapeo físico:

\begin{table}[H]
	\centering
	\begin{tabular}{
		l
		c
		l
	}
		\toprule
		\textbf{Gesto reconocido} &
		\textbf{Ángulo del servo} &
		\textbf{Posición} \\
		\midrule

		\textit{right} &
		$0^{\circ}$ &
		Derecha \\

		\textit{up} &
		$90^{\circ}$ &
		Posición central o superior \\

		\textit{left} &
		$180^{\circ}$ &
		Izquierda \\

		\bottomrule
	\end{tabular}
	\caption{Posición del servomotor asociada a cada gesto.}
	\label{tab:servo}
\end{table}

\paragraph{\textit{Alimentación del servomotor.}}

El servomotor no debe alimentarse desde el pin de 3,3 V del ESP32. Se debe
utilizar una fuente de 5 V adecuada y conectar la tierra de dicha fuente con
la tierra del ESP32. Esta conexión común es necesaria para que la señal PWM
tenga una referencia eléctrica correcta.


% =========================================================
% FIRMWARE
% =========================================================

\section{\textit{Diseño del firmware del ESP32}}

\subsection{\textit{Inicialización}}

Durante el arranque, el firmware configura:

\begin{itemize}
	\item Puerto serial a 115200 baudios.
	\item Pines del LED y del buzzer.
	\item Señal PWM del servomotor.
	\item Pantalla OLED mediante SPI.
	\item Bus I2C a 100 kHz.
	\item MPU6050.
	\item APDS-9960 como referencia opcional.
	\item Conexión WiFi.
	\item Pulsador e interrupción.
\end{itemize}

La pantalla OLED informa al usuario el progreso del proceso de
inicialización.

\subsection{\textit{Solicitud de captura mediante interrupción}}

El pulsador se conecta entre GPIO27 y tierra. El pin se configura mediante:

\begin{center}
	\texttt{INPUT\_PULLUP}
\end{center}

La interrupción se activa por flanco descendente. La rutina de interrupción
únicamente establece una bandera de solicitud de captura. La adquisición, la
construcción del JSON y la comunicación HTTP se ejecutan por fuera de la
rutina de interrupción.

Esta decisión evita operaciones lentas dentro de la interrupción y reduce el
riesgo de bloqueos o comportamientos impredecibles.

\subsection{\textit{Adquisición de las señales inerciales}}

Cada captura contiene una secuencia de 100 muestras. La muestra $n$ se
representa mediante:

\begin{equation}
	\mathbf{x}[n] =
	\begin{bmatrix}
		a_x[n] &
		a_y[n] &
		a_z[n] &
		g_x[n] &
		g_y[n] &
		g_z[n]
	\end{bmatrix},
	\qquad
	n=0,\ldots,99.
\end{equation}

El periodo nominal es:

\begin{equation}
	T_s = 20\ \text{ms}.
\end{equation}

La frecuencia de muestreo correspondiente es:

\begin{equation}
	f_s = \frac{1}{T_s} = 50\ \text{Hz}.
\end{equation}

La duración nominal de la ventana es:

\begin{equation}
	T_{\mathrm{captura}}
	=
	100 \times 20\ \text{ms}
	\approx
	2\ \text{s}.
\end{equation}

\subsection{\textit{Validación del movimiento}}

Una prueba basada exclusivamente en verificar que los datos fueran
diferentes de cero no resultó suficiente, debido al ruido natural del
MPU6050. Por esta razón se implementó una validación basada en el valor pico
a pico.

Para un canal cualquiera:

\begin{equation}
	\Delta x = x_{\max} - x_{\min}.
\end{equation}

Se calcula el máximo pico a pico de los canales del acelerómetro:

\begin{equation}
	A_{\mathrm{pp,max}}
	=
	\max
	\left(
	\Delta a_x,
	\Delta a_y,
	\Delta a_z
	\right).
\end{equation}

De manera equivalente, para el giróscopo:

\begin{equation}
	G_{\mathrm{pp,max}}
	=
	\max
	\left(
	\Delta g_x,
	\Delta g_y,
	\Delta g_z
	\right).
\end{equation}

La captura se acepta cuando:

\begin{equation}
	A_{\mathrm{pp,max}} \geq 1500
	\quad
	\lor
	\quad
	G_{\mathrm{pp,max}} \geq 500.
\end{equation}

Si ninguna condición se cumple, la pantalla muestra
\texttt{MOTION REJECTED} y los datos no se transmiten a la Raspberry Pi.

\subsection{\textit{Comunicación inalámbrica}}

Las capturas aceptadas se codifican mediante la siguiente estructura JSON:

\begin{verbatim}
{
  "samples": [
    {
      "ax": ...,
      "ay": ...,
      "az": ...,
      "gx": ...,
      "gy": ...,
      "gz": ...
    }
  ]
}
\end{verbatim}

El mensaje se envía mediante una solicitud HTTP POST al siguiente recurso:

\begin{center}
	\texttt{http://172.20.10.7:5000/api/gesture}
\end{center}

La respuesta del servidor contiene, entre otros, los siguientes campos:

\begin{itemize}
	\item \texttt{success}.
	\item \texttt{gesture}.
	\item \texttt{confidence}.
	\item \texttt{probabilities}.
	\item \texttt{received\_samples}.
	\item \texttt{model\_engine}.
	\item \texttt{inference\_time\_ms}.
\end{itemize}


% =========================================================
% DATASET E IA
% =========================================================

\section{\textit{Conjunto de datos e inteligencia artificial}}

\subsection{\textit{Recolección del conjunto de datos}}

El conjunto de datos real se construyó mediante capturas simultáneas del
MPU6050 y del APDS-9960. El MPU6050 suministró las señales de entrada,
mientras que el APDS-9960 se utilizó para determinar la etiqueta de
referencia.

El conjunto final contiene tres clases:

\begin{itemize}
	\item \textit{left}.
	\item \textit{right}.
	\item \textit{up}.
\end{itemize}

\begin{table}[H]
	\centering
	\begin{tabular}{
		l
		c
	}
		\toprule
		\textbf{Parámetro} &
		\textbf{Valor} \\
		\midrule

		Número de clases &
		3 \\

		Capturas por clase &
		40 \\

		Total de capturas &
		120 \\

		Muestras por captura &
		100 \\

		Canales por muestra &
		6 \\

		Frecuencia de muestreo &
		50 Hz \\

		Duración aproximada &
		2 s \\

		Forma de entrada &
		$(120,100,6)$ \\

		\bottomrule
	\end{tabular}
	\caption{Configuración del conjunto de datos real.}
	\label{tab:dataset}
\end{table}

\subsection{\textit{División del conjunto de datos}}

Se realizó una división estratificada con semilla aleatoria 42:

\begin{itemize}
	\item Entrenamiento: 72 capturas, 24 por clase.
	\item Validación: 24 capturas, 8 por clase.
	\item Prueba: 24 capturas, 8 por clase.
\end{itemize}

\subsection{\textit{Normalización}}

Los parámetros de normalización se calcularon utilizando únicamente el
conjunto de entrenamiento. Esto evita que el conjunto de prueba suministre
información durante el entrenamiento.

Cada canal se normaliza de acuerdo con:

\begin{equation}
	\hat{x}_{c}
	=
	\frac{x_c-\mu_c}{\sigma_c},
\end{equation}

donde $\mu_c$ representa la media y $\sigma_c$ la desviación estándar del
canal $c$ calculadas sobre los datos de entrenamiento.

\subsection{\textit{Arquitectura de la red neuronal}}

La red utiliza convoluciones temporales para extraer patrones locales de la
secuencia inercial.

\begin{table}[H]
	\centering
	\begin{tabularx}{\textwidth}{
		p{0.28\textwidth}
		p{0.26\textwidth}
		L
	}
		\toprule
		\textbf{Capa} &
		\textbf{Configuración} &
		\textbf{Función} \\
		\midrule

		Entrada &
		$100 \times 6$ &
		Secuencia inercial de aproximadamente dos segundos. \\

		Conv1D &
		16 filtros, kernel 5, ReLU &
		Extracción inicial de características temporales. \\

		MaxPooling1D &
		Tamaño 2 &
		Reducción de la dimensión temporal. \\

		Conv1D &
		32 filtros, kernel 3, ReLU &
		Extracción de características de mayor nivel. \\

		GlobalAveragePooling1D &
		Promedio global &
		Conversión de cada mapa de características en un escalar. \\

		Dense &
		16 unidades, ReLU &
		Representación no lineal de las características. \\

		Salida &
		3 unidades, Softmax &
		Probabilidades para \textit{left}, \textit{right} y
		\textit{up}. \\

		\bottomrule
	\end{tabularx}
	\caption{Arquitectura de la red neuronal convolucional 1D.}
	\label{tab:cnn}
\end{table}

El modelo contiene un total de 2643 parámetros entrenables.

\subsection{\textit{Entrenamiento}}

El entrenamiento utilizó detención temprana para evitar un sobreajuste
excesivo. El mejor resultado se obtuvo en la época 33, aunque el proceso
finalizó en la época 58.

Los valores aproximados al finalizar el proceso fueron:

\begin{itemize}
	\item Exactitud de entrenamiento: 91,67\,\%.
	\item Exactitud de validación: 87,5\,\%.
	\item Exactitud de prueba: 83,33\,\%.
\end{itemize}

\subsection{\textit{Implementación de la inferencia con NumPy}}

El modelo fue exportado en formatos Keras, TensorFlow Lite y NumPy. Debido a
las limitaciones de la Raspberry Pi Zero W, la versión final no utiliza
TensorFlow durante la inferencia.

Se implementaron mediante NumPy las siguientes operaciones:

\begin{itemize}
	\item Convolución unidimensional.
	\item Función de activación ReLU.
	\item Max pooling.
	\item Promedio global.
	\item Capas densas.
	\item Función Softmax.
\end{itemize}

La diferencia numérica máxima observada entre la implementación de Keras y
la implementación manual de NumPy fue aproximadamente:

\begin{equation}
	1,788 \times 10^{-7}.
\end{equation}

Esta diferencia se considera despreciable y demuestra la equivalencia
práctica entre ambas implementaciones.


% =========================================================
% RASPBERRY E IOT
% =========================================================

\section{\textit{Software de la Raspberry Pi e interfaz IoT}}

\subsection{\textit{Servidor Flask}}

El servidor se ejecuta desde:

\begin{center}
	\texttt{/home/salo/embedded\_gesture\_system/app.py}
\end{center}

Los recursos principales de la aplicación son:

\begin{itemize}
	\item \texttt{GET /}: panel web.
	\item \texttt{GET /api/status}: estado actual del sistema.
	\item \texttt{POST /api/dataset}: almacenamiento de capturas etiquetadas.
	\item \texttt{POST /api/gesture}: clasificación de una captura.
\end{itemize}

\subsection{\textit{Procesamiento de una solicitud}}

Cuando el servidor recibe una solicitud en
\texttt{/api/gesture}, realiza las siguientes operaciones:

\begin{enumerate}
	\item Verifica que el cuerpo de la solicitud sea JSON.
	\item Verifica que existan exactamente 100 muestras.
	\item Comprueba la presencia de los seis canales.
	\item Convierte los datos a un arreglo de tipo
	\texttt{float32}.
	\item Normaliza cada canal.
	\item Ejecuta la CNN mediante NumPy.
	\item Calcula la clase y la confianza.
	\item Retorna el resultado al ESP32.
	\item Actualiza el estado mostrado por el panel web.
\end{enumerate}

\subsection{\textit{Inicio automático}}

El servidor se administra mediante el servicio:

\begin{center}
	\texttt{gesture-server.service}
\end{center}

El servicio se encuentra habilitado para iniciar automáticamente después del
arranque de la Raspberry. De esta forma, no es necesario ejecutar
manualmente el archivo \texttt{app.py} después de cada reinicio
\cite{rpi_inicio}.

\subsection{\textit{Configuración de red}}

La Raspberry utiliza el perfil:

\begin{center}
	\texttt{gesture-hotspot-static}
\end{center}

La configuración validada incluye:

\begin{itemize}
	\item Dirección IP fija \texttt{172.20.10.7}.
	\item Conexión automática habilitada.
	\item Prioridad de conexión igual a 200.
	\item Ahorro de energía WiFi deshabilitado.
	\item Inicio automático del servidor Flask.
\end{itemize}

\subsection{\textit{Panel web}}

El panel web consulta periódicamente el recurso
\texttt{/api/status}. La interfaz presenta:

\begin{itemize}
	\item Estado del sistema.
	\item Último gesto reconocido.
	\item Confianza de la predicción.
	\item Número de muestras recibidas.
	\item Hora de la última actualización.
\end{itemize}

La interfaz web complementa la pantalla OLED. La OLED proporciona
información local junto al montaje, mientras que el panel permite observar
la inferencia desde un navegador conectado a la misma red.


% =========================================================
% RESULTADOS
% =========================================================

\section{\textit{Resultados experimentales}}

\subsection{\textit{Evaluación del modelo}}

El modelo obtuvo 20 clasificaciones correctas sobre las 24 muestras del
conjunto de prueba:

\begin{equation}
	\mathrm{Exactitud}
	=
	\frac{20}{24}
	\times
	100\,\%
	=
	83,33\,\%.
\end{equation}

La matriz de confusión obtenida fue:

\begin{equation}
	\mathbf{C}
	=
	\begin{bmatrix}
		8 & 0 & 0 \\
		0 & 6 & 2 \\
		0 & 2 & 6
	\end{bmatrix}.
\end{equation}

\begin{table}[H]
	\centering
	\begin{tabular}{
		l
		c
		c
	}
		\toprule
		\textbf{Clase} &
		\textbf{Aciertos} &
		\textbf{Exactitud} \\
		\midrule

		\textit{left} &
		8 de 8 &
		100\,\% \\

		\textit{right} &
		6 de 8 &
		75\,\% \\

		\textit{up} &
		6 de 8 &
		75\,\% \\

		\bottomrule
	\end{tabular}
	\caption{Resultados por clase sobre el conjunto de prueba.}
	\label{tab:resultados_prueba}
\end{table}

La mayor confusión se presentó entre los gestos \textit{right} y
\textit{up}. La clase \textit{left} fue reconocida correctamente en todas
las muestras del conjunto de prueba.

\subsection{\textit{Validación de extremo a extremo}}

Se validó el siguiente recorrido completo:

\begin{center}
	Pulsador
	$\rightarrow$
	MPU6050
	$\rightarrow$
	ESP32
	$\rightarrow$
	WiFi
	$\rightarrow$
	Flask
	$\rightarrow$
	CNN con NumPy
	$\rightarrow$
	respuesta al ESP32.
\end{center}

Algunas pruebas en vivo exitosas se presentan en la
Tabla~\ref{tab:pruebas_vivo}.

\begin{table}[H]
	\centering
	\begin{tabular}{
		l
		l
		c
	}
		\toprule
		\textbf{Gesto realizado} &
		\textbf{Gesto predicho} &
		\textbf{Confianza} \\
		\midrule

		\textit{left} &
		\textit{left} &
		99,01\,\% \\

		\textit{right} &
		\textit{right} &
		93,57\,\% \\

		\textit{up} &
		\textit{up} &
		91,18\,\% \\

		\bottomrule
	\end{tabular}
	\caption{Resultados seleccionados de inferencia en vivo.}
	\label{tab:pruebas_vivo}
\end{table}

En otra prueba utilizada para validar el panel web se reconoció el gesto
\textit{left} con una confianza del 60,75\,\% y un tiempo de inferencia
aproximado de 117,6 ms.

La reducción de confianza en esta prueba muestra que el resultado depende de
la forma específica de ejecutar el movimiento, la orientación inicial y la
similitud entre el gesto realizado y los ejemplos utilizados durante el
entrenamiento.

\subsection{\textit{Rechazo de capturas estacionarias}}

Durante una prueba sin movimiento, los valores pico a pico se encontraron
por debajo de los umbrales configurados. El firmware rechazó la captura y no
envió información a la Raspberry Pi.

Este comportamiento evita que el ruido natural del sensor sea interpretado
por la CNN como un gesto válido.

\subsection{\textit{Estado de integración de la OLED y el servo}}

La pantalla OLED SPI y el servomotor SG90 fueron validados inicialmente
mediante un sketch conjunto. En dicha prueba:

\begin{itemize}
	\item La pantalla mostró dinámicamente el ángulo y la dirección.
	\item El servomotor realizó recorridos entre 0 y 180 grados.
	\item La posición de 0 grados se asoció con \textit{right}.
	\item La posición de 90 grados se asoció con \textit{up}.
	\item La posición de 180 grados se asoció con \textit{left}.
\end{itemize}

Posteriormente, ambos componentes fueron incorporados en una versión del
firmware de inferencia que compiló correctamente. La validación final debe
comprobar que la pantalla muestre todos los estados y que el servo responda a
las tres clases sin causar reinicios ni alterar la captura.

\subsection{\textit{Medidas de confiabilidad}}

\begin{itemize}
	\item Rechazo local de capturas estacionarias.
	\item Dirección IP fija para la Raspberry Pi.
	\item Ahorro de energía WiFi deshabilitado.
	\item Inicio automático del servidor mediante
	\texttt{systemd}.
	\item Respaldo del firmware estable.
	\item Respaldo del conjunto de datos real.
	\item Respaldo del modelo y del notebook de entrenamiento.
	\item Verificación de archivos mediante hashes SHA256.
\end{itemize}


% =========================================================
% PROCEDIMIENTO DE DEMOSTRACIÓN
% =========================================================

\section{\textit{Procedimiento de demostración}}

\subsection{\textit{Preparación}}

Antes de realizar la demostración se debe:

\begin{enumerate}
	\item Activar el punto de acceso WiFi de 2,4 GHz.
	\item Conectar el computador a la misma red.
	\item Energizar la Raspberry Pi.
	\item Esperar el arranque del sistema operativo.
	\item Verificar el acceso a
	\texttt{http://172.20.10.7:5000}.
	\item Conectar el ESP32 y abrir el monitor serial.
	\item Confirmar que la OLED indique
	\texttt{SYSTEM READY}.
	\item Confirmar que el servo se encuentre en su posición inicial.
\end{enumerate}

\subsection{\textit{Ejecución de un gesto}}

Para ejecutar una inferencia:

\begin{enumerate}
	\item Presionar una vez el pulsador.
	\item Esperar la indicación de preparación.
	\item Realizar el gesto durante la ventana de captura.
	\item Mantener la orientación empleada durante la recolección del
	conjunto de datos.
	\item Esperar el envío y procesamiento.
	\item Verificar la clase y la confianza en la OLED.
	\item Verificar el movimiento del servo.
	\item Confirmar el mismo resultado en el panel web.
\end{enumerate}

\subsection{\textit{Secuencia recomendada}}

La demostración debe incluir:

\begin{itemize}
	\item Un gesto \textit{left}.
	\item Un gesto \textit{right}.
	\item Un gesto \textit{up}.
	\item Una captura estacionaria rechazada.
	\item Visualización de los resultados en el panel web.
	\item Evidencia del movimiento del servomotor.
\end{itemize}


% =========================================================
% LIMITACIONES
% =========================================================

\section{\textit{Limitaciones y trabajo futuro}}

\subsection{\textit{Limitaciones actuales}}

\begin{itemize}
	\item El conjunto de datos contiene únicamente 120 capturas.
	\item La mayoría de las capturas fueron realizadas por un número
	limitado de personas.
	\item La exactitud se evaluó sobre solo 24 ejemplos de prueba.
	\item Existe confusión entre los gestos \textit{right} y
	\textit{up}.
	\item La ejecución del gesto depende de la orientación inicial del
	sensor.
	\item El sistema depende de una red WiFi compartida.
	\item El punto de acceso de un teléfono no constituye una
	infraestructura industrial.
	\item El servo puede introducir ruido eléctrico si su alimentación no
	es estable.
	\item El prototipo se encuentra montado sobre protoboard.
	\item No se fabricó una tarjeta PCB propia para la versión presentada.
\end{itemize}

\subsection{\textit{Trabajo futuro}}

\begin{itemize}
	\item Aumentar el número de capturas del conjunto de datos.
	\item Incluir movimientos realizados por diferentes usuarios.
	\item Recolectar ejemplos con distintas velocidades y orientaciones.
	\item Implementar técnicas de aumento de datos.
	\item Evaluar modelos temporales alternativos.
	\item Cuantizar el modelo para reducir memoria y tiempo de inferencia.
	\item Implementar la inferencia directamente en un microcontrolador
	más reciente.
	\item Incorporar un historial persistente en el panel web.
	\item Diseñar un esquemático completo en KiCad.
	\item Diseñar y fabricar una tarjeta PCB.
	\item Construir una carcasa para el dispositivo.
	\item Mejorar la recuperación automática después de una pérdida de
	red.
\end{itemize}


% =========================================================
% CONCLUSIONES
% =========================================================

\section{\textit{Conclusiones}}

Se desarrolló un sistema embebido inalámbrico capaz de adquirir señales
inerciales, transmitirlas por WiFi, ejecutar una red neuronal y generar una
respuesta física y visual.

El ESP32 permitió integrar la adquisición del MPU6050, la interrupción del
pulsador, la validación de movimiento, la comunicación inalámbrica y el
control de los actuadores. La Raspberry Pi Zero W proporcionó un entorno
Linux embebido para ejecutar el servidor Flask y la inferencia de la red
neuronal.

El conjunto de datos estuvo compuesto por 120 capturas balanceadas. La red
neuronal convolucional 1D, con 2643 parámetros, alcanzó una exactitud de
prueba del 83,33\,\%. La clase \textit{left} obtuvo una exactitud del
100\,\%, mientras que las clases \textit{right} y \textit{up} obtuvieron
75\,\%.

La implementación manual mediante NumPy reprodujo las salidas de Keras con
una diferencia numérica despreciable. Esto permitió desplegar la CNN en una
Raspberry Pi Zero W a pesar de no disponer de TensorFlow Lite Runtime.

Las pruebas de extremo a extremo confirmaron la comunicación entre el ESP32
y la Raspberry, la recepción de las 100 muestras, la ejecución del modelo y
la devolución del resultado. El rechazo de capturas estacionarias mejoró el
comportamiento del sistema al impedir inferencias sobre señales que
contenían únicamente ruido.

La pantalla OLED dinámica permite observar localmente el estado del proceso,
mientras que el panel web proporciona una visualización remota. El
servomotor transforma la clase predicha en una respuesta mecánica asociada
con la dirección del gesto.

En conjunto, el proyecto integra sensores, actuadores, interrupciones,
firmware, Linux embebido, comunicación WiFi, servicios IoT e inteligencia
artificial en un prototipo funcional.


% =========================================================
% REFERENCIAS
% =========================================================

\begin{thebibliography}{99}

	\bibitem{unal_programa}
	Universidad Nacional de Colombia, Departamento de Ingeniería Eléctrica
	y Electrónica, ``Programa de Sistemas Embebidos, semestre 1-2026,''
	Bogotá, Colombia, 2026.

	\bibitem{gallego_componentes}
	A. Gallego, ``Componentes de un Sistema Embebido,'' material de clase,
	Departamento de Ingeniería Eléctrica y Electrónica, Universidad
	Nacional de Colombia, febrero de 2026.

	\bibitem{gallego_ia}
	A. Gallego, ``Conceptos básicos de IA y Microcontroladores,'' material
	de clase, Departamento de Ingeniería Eléctrica y Electrónica,
	Universidad Nacional de Colombia, marzo de 2026.

	\bibitem{espressif}
	Espressif Systems, ``ESP32 Series Datasheet,'' versión 4.3, 2024.

	\bibitem{mpu6050}
	InvenSense Inc., ``MPU-6000 and MPU-6050 Product Specification,''
	revisión 3.4, Sunnyvale, California, Estados Unidos, 2013.

	\bibitem{raspberry}
	Raspberry Pi Ltd., ``Raspberry Pi Zero W Product Brief,'' Cambridge,
	Reino Unido.

	\bibitem{tinyml}
	P. Warden y D. Situnayake,
	\textit{TinyML: Machine Learning with TensorFlow Lite on Arduino and
	Ultra-Low-Power Microcontrollers}.
	Sebastopol, California, Estados Unidos: O'Reilly Media, 2020.

	\bibitem{cavagnis}
	L. Cavagnis, ``TinyML: Machine Learning for Embedded System---Part II,''
	\textit{Medium}, diciembre de 2024.

	\bibitem{rpi_inicio}
	S. Kildall, ``Raspberry Pi: Launch Python Script on Startup,''
	\textit{Instructables}, consultado en julio de 2026.

	\bibitem{flask}
	Pallets Projects, ``Flask Documentation,'' documentación de software.
	Disponible en: \url{https://flask.palletsprojects.com/}

	\bibitem{adafruit}
	Adafruit Industries, ``Adafruit SSD1306 and Adafruit GFX Library
	Documentation,'' documentación de software.

	\bibitem{esp32servo}
	K. Harrington y J. K. Bennett, ``ESP32Servo Library,'' biblioteca de
	software.

\end{thebibliography}

\end{document}

Ya cargué el firmware en la esp e hice la prueba y salió esto. El gesto que hice sí fue left. Hay un problema que viene desde hace varias versiones del firmware: 3 de cada 5 veces, presiono una vez el botón, y al terminar una captura, inmediatamente toma otra como si hubiera vuelto a presionar el botón, tomando otra captura. También pasa que es muy común que salga el error de que la lectura del MPU falló en la muestra x. A veces la muestra 0, a veces cualquier otra.

CNN INFERENCE RESULT
Predicted gesture: left
Confidence: 0.9921
Probabilities:
  left: 0.9921
  right: 0.0019
  up: 0.0060
APDS reference comparison: unavailable

INFERENCE COMPLETED SUCCESSFULLY.

Worked for 4m 7s
El resultado de inferencia está perfecto: left con 99,21 %. La CNN no es el problema. Los dos fallos vienen del firmware, concretamente de una lógica de botón demasiado optimista y una lectura I2C sin tolerancia a errores. El software asumía que tanto el pulsador como el bus I2C serían ciudadanos ejemplares. Tierna fantasía.

Por qué ocurría la doble captura
La interrupción del botón permanecía habilitada durante toda la secuencia:

captura → HTTP → CNN → OLED → servo → resultado
Durante esos segundos, un rebote del pulsador o ruido eléctrico podía volver a poner:

captureRequested = true;
Cuando terminaba la primera inferencia, el tiempo de antirrebote de 500 ms ya había vencido hacía rato. El programa encontraba esa solicitud pendiente y arrancaba inmediatamente otra captura.

La corrección implementada:

Deshabilita la interrupción mientras el sistema está ocupado.

Ignora rebotes y ruido durante captura, comunicación y actuación.

Espera a que el botón se libere físicamente.

Espera otros 80 ms de estabilidad.

Limpia cualquier solicitud pendiente.

Reactiva la interrupción únicamente cuando el sistema está listo.

Por qué fallaba una muestra del MPU6050
El firmware anterior cancelaba toda la captura ante un solo fallo transitorio de I2C. Bastaba con que una lectura devolviera menos de 14 bytes una vez para abortar, aunque las otras 99 lecturas hubieran sido correctas.

La versión corregida:

Reintenta cada muestra hasta cuatro veces.

Limpia bytes pendientes del búfer I2C.

Después de dos fallos consecutivos reinicia el controlador I2C del ESP32.

Solo cancela la captura si los cuatro intentos fallan.

Informa cuántas lecturas fueron recuperadas, sin imprimir durante el muestreo ni alterar los 50 Hz.

Preparé el firmware corregido:

Descargar firmware corregido v2

Archivo individual:

gesture_inference_wifi_oled_servo_v2.ino

SHA256 del sketch:

E102796002AD6527685030EE389A78A61852B3B6D61CF4B3C487A38C2E8C65ED
Instalar la versión v2
En Windows PowerShell, ejecuta:

$projectPath = "C:\Users\luisfer\iCloudDrive\U\SISTEMAS EMBEBIDOS\PROYECTO"

$zipFile = Get-ChildItem `
    -Path "$HOME\Downloads" `
    -Filter "gesture_inference_wifi_oled_servo_v2*.zip" |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

$esp32Path = Join-Path $projectPath "ESP32"

Expand-Archive `
    -Path $zipFile.FullName `
    -DestinationPath $esp32Path `
    -Force

$oldSecrets = Join-Path $esp32Path "gesture_inference_wifi_oled_servo_v1\secrets.h"
$newFolder = Join-Path $esp32Path "gesture_inference_wifi_oled_servo_v2"
$newSecrets = Join-Path $newFolder "secrets.h"
$newSketch = Join-Path $newFolder "gesture_inference_wifi_oled_servo_v2.ino"

Copy-Item `
    -Path $oldSecrets `
    -Destination $newSecrets `
    -Force

Write-Host "SKETCH_EXISTS=$(Test-Path $newSketch)"
Write-Host "SECRETS_EXISTS=$(Test-Path $newSecrets)"
Write-Host "SKETCH_SHA256=$((Get-FileHash -Algorithm SHA256 $newSketch).Hash)"
Luego abre:

C:\Users\luisfer\iCloudDrive\U\SISTEMAS EMBEBIDOS\PROYECTO\ESP32\gesture_inference_wifi_oled_servo_v2\gesture_inference_wifi_oled_servo_v2.ino
En Arduino IDE:

Board: DOIT ESP32 DEVKIT V1
Port: COM3
Serial Monitor: 115200
Pulsa Verify y después Upload.

Prueba exacta
Haz cinco pruebas, pulsando una sola vez en cada una. Después de cada resultado espera a que la OLED vuelva a SYSTEM READY.

Una captura correcta mostrará además:

Recovered MPU6050 reads: 0 (maximum attempts used: 1)
También puede aparecer algo como:

Recovered MPU6050 reads: 2 (maximum attempts used: 2)
Eso no es un error. Significa que dos lecturas I2C fallaron inicialmente, pero el firmware las recuperó y completó las 100 muestras.

El único mensaje realmente problemático sería:

ERROR: MPU6050 read failed at sample X after 4 attempts.
Si aparece incluso con esta versión, el siguiente ajuste será físico: revisar los jumpers SDA/SCL y colocar un condensador de 470 µF a 1000 µF en la alimentación de 5 V del servo. Pero primero probamos la corrección lógica, que ataca exactamente los dos defectos que describiste.


gesture_inference_wifi_oled_servo_v2.ino
File

También hay un problema que ya se había corregido previamente pero regresó, y es que al dejar el mpu quieto, igual se interpreta un gesto.


Library
/
Embedded Systems
/
gesture_inference_wifi_oled_servo_v2.ino


#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include <SparkFun_APDS9960.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#include "secrets.h"

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint32_t I2C_CLOCK_HZ = 100000;

constexpr uint8_t MPU6050_ADDRESS = 0x68;
constexpr uint8_t MPU6050_SIGNAL_PATH_RESET = 0x68;
constexpr uint8_t MPU6050_PWR_MGMT_1 = 0x6B;
constexpr uint8_t MPU6050_PWR_MGMT_2 = 0x6C;
constexpr uint8_t MPU6050_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t MPU6050_WHO_AM_I = 0x75;

constexpr uint8_t BUTTON_PIN = 27;
constexpr uint8_t LED_PIN = 26;

constexpr int16_t SCREEN_WIDTH = 128;
constexpr int16_t SCREEN_HEIGHT = 64;

constexpr int8_t OLED_MOSI_PIN = 23;
constexpr int8_t OLED_CLK_PIN = 18;
constexpr int8_t OLED_DC_PIN = 2;
constexpr int8_t OLED_CS_PIN = 5;
constexpr int8_t OLED_RESET_PIN = 4;

constexpr uint8_t SERVO_PIN = 13;
constexpr int SERVO_RIGHT_ANGLE = 0;
constexpr int SERVO_UP_ANGLE = 90;
constexpr int SERVO_LEFT_ANGLE = 180;
constexpr int SERVO_INITIAL_ANGLE = SERVO_UP_ANGLE;
constexpr uint16_t SERVO_STEP_DELAY_MS = 8;

constexpr uint32_t RESULT_SCREEN_TIME_MS = 2200;
constexpr uint32_t PROBABILITY_SCREEN_TIME_MS = 2200;
constexpr uint32_t MESSAGE_SCREEN_TIME_MS = 1800;

constexpr size_t SAMPLE_COUNT = 100;
constexpr uint32_t SAMPLE_INTERVAL_US = 20000;

constexpr uint32_t BUTTON_DEBOUNCE_MS = 350;
constexpr uint32_t BUTTON_RELEASE_STABLE_MS = 80;
constexpr uint8_t MPU_READ_MAX_ATTEMPTS = 4;
constexpr uint32_t MPU_READ_RETRY_DELAY_US = 500;
constexpr uint32_t WIFI_CONNECTION_TIMEOUT_MS = 20000;
constexpr uint32_t HTTP_REQUEST_TIMEOUT_MS = 20000;
constexpr uint32_t GESTURE_WAIT_TIMEOUT_MS = 1500;

constexpr int32_t MIN_ACCEL_PEAK_TO_PEAK = 1500;
constexpr int32_t MIN_GYRO_PEAK_TO_PEAK = 500;

struct MotionSample
{
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;
};

struct InferenceResult
{
  bool success;
  String gesture;
  float confidence;
  float probabilityLeft;
  float probabilityRight;
  float probabilityUp;
};

SparkFun_APDS9960 apds9960;

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  OLED_MOSI_PIN,
  OLED_CLK_PIN,
  OLED_DC_PIN,
  OLED_RESET_PIN,
  OLED_CS_PIN
);

Servo gestureServo;

bool displayAvailable = false;
bool servoAvailable = false;
int currentServoAngle = SERVO_INITIAL_ANGLE;

MotionSample samples[SAMPLE_COUNT];

volatile bool captureRequested = false;

uint32_t lastCaptureMs = 0;

void IRAM_ATTR handleButtonInterrupt()
{
  captureRequested = true;
}

void drawCenteredText(
  const String &text,
  int16_t y,
  uint8_t textSize
)
{
  if (!displayAvailable)
  {
    return;
  }

  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t width = 0;
  uint16_t height = 0;

  display.setTextSize(textSize);
  display.getTextBounds(
    text,
    0,
    y,
    &x1,
    &y1,
    &width,
    &height
  );

  int16_t x =
    (
      SCREEN_WIDTH -
      static_cast<int16_t>(width)
    ) / 2;

  if (x < 0)
  {
    x = 0;
  }

  display.setCursor(x, y);
  display.print(text);
}

void showStateScreen(
  const String &title,
  const String &line1 = "",
  const String &line2 = "",
  const String &line3 = ""
)
{
  if (!displayAvailable)
  {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  drawCenteredText(title, 2, 1);
  display.drawFastHLine(
    0,
    13,
    SCREEN_WIDTH,
    SSD1306_WHITE
  );

  drawCenteredText(line1, 20, 1);
  drawCenteredText(line2, 34, 1);
  drawCenteredText(line3, 48, 1);

  display.display();
}

void showReadyScreen()
{
  const String wifiLine =
    WiFi.status() == WL_CONNECTED
      ? "WiFi: connected"
      : "WiFi: retry later";

  showStateScreen(
    "SYSTEM READY",
    "Press the button",
    "to capture gesture",
    wifiLine
  );
}

void showPrepareCountdown()
{
  for (int countdown = 3; countdown >= 1; countdown--)
  {
    showStateScreen(
      "PREPARE GESTURE",
      "Starting in",
      String(countdown),
      "Move during capture"
    );

    digitalWrite(LED_PIN, HIGH);
    delay(150);

    digitalWrite(LED_PIN, LOW);
    delay(150);
  }

  delay(100);
}

void showResultSummary(
  const InferenceResult &result,
  int servoAngle
)
{
  if (!displayAvailable)
  {
    return;
  }

  String gestureText = result.gesture;
  gestureText.toUpperCase();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  drawCenteredText("GESTURE RESULT", 2, 1);
  display.drawFastHLine(
    0,
    13,
    SCREEN_WIDTH,
    SSD1306_WHITE
  );

  drawCenteredText(gestureText, 18, 2);

  drawCenteredText(
    String("Confidence: ") +
    String(result.confidence * 100.0F, 2) +
    "%",
    41,
    1
  );

  const String servoLine =
    servoAngle >= 0
      ? String("Servo: ") +
        String(servoAngle) +
        " deg"
      : String("Servo: no command");

  drawCenteredText(servoLine, 53, 1);

  display.display();
}

void showProbabilityScreen(
  const InferenceResult &result
)
{
  showStateScreen(
    "CNN PROBABILITIES",
    String("Left:  ") +
      String(result.probabilityLeft * 100.0F, 2) +
      "%",
    String("Right: ") +
      String(result.probabilityRight * 100.0F, 2) +
      "%",
    String("Up:    ") +
      String(result.probabilityUp * 100.0F, 2) +
      "%"
  );
}

int gestureToServoAngle(const String &gesture)
{
  if (gesture == "right")
  {
    return SERVO_RIGHT_ANGLE;
  }

  if (gesture == "up")
  {
    return SERVO_UP_ANGLE;
  }

  if (gesture == "left")
  {
    return SERVO_LEFT_ANGLE;
  }

  return -1;
}

int moveServoForGesture(const String &gesture)
{
  const int targetAngle =
    gestureToServoAngle(gesture);

  if (
    !servoAvailable ||
    targetAngle < 0
  )
  {
    return -1;
  }

  const int step =
    targetAngle >= currentServoAngle
      ? 1
      : -1;

  int angle = currentServoAngle;

  while (angle != targetAngle)
  {
    gestureServo.write(angle);
    angle += step;
    delay(SERVO_STEP_DELAY_MS);
  }

  gestureServo.write(targetAngle);
  currentServoAngle = targetAngle;

  return targetAngle;
}

bool writeMpuRegister(
  uint8_t registerAddress,
  uint8_t value
)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}

bool readMpuRegister(
  uint8_t registerAddress,
  uint8_t &value
)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  if (
    Wire.requestFrom(
      MPU6050_ADDRESS,
      static_cast<uint8_t>(1)
    ) != 1
  )
  {
    return false;
  }

  value = Wire.read();

  return true;
}

bool initializeMpu6050()
{
  if (!writeMpuRegister(MPU6050_PWR_MGMT_1, 0x80))
  {
    return false;
  }

  delay(100);

  if (
    !writeMpuRegister(
      MPU6050_SIGNAL_PATH_RESET,
      0x07
    )
  )
  {
    return false;
  }

  delay(100);

  if (!writeMpuRegister(MPU6050_PWR_MGMT_1, 0x01))
  {
    return false;
  }

  delay(20);

  if (!writeMpuRegister(MPU6050_PWR_MGMT_2, 0x00))
  {
    return false;
  }

  delay(20);

  uint8_t deviceId = 0;

  if (!readMpuRegister(MPU6050_WHO_AM_I, deviceId))
  {
    return false;
  }

  Serial.printf(
    "MPU6050 device ID: 0x%02X\n",
    deviceId
  );

  return deviceId != 0x00 && deviceId != 0xFF;
}

bool readMpuSampleOnce(MotionSample &sample)
{
  while (Wire.available())
  {
    Wire.read();
  }

  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(MPU6050_ACCEL_XOUT_H);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  const size_t receivedBytes =
    Wire.requestFrom(
      MPU6050_ADDRESS,
      static_cast<uint8_t>(14)
    );

  if (receivedBytes != 14 || Wire.available() < 14)
  {
    while (Wire.available())
    {
      Wire.read();
    }

    return false;
  }

  sample.ax =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.ay =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.az =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  Wire.read();
  Wire.read();

  sample.gx =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.gy =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.gz =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  return true;
}

void recoverI2cController()
{
  while (Wire.available())
  {
    Wire.read();
  }

  Wire.end();
  delay(2);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);
}

bool readMpuSample(
  MotionSample &sample,
  uint8_t &attemptsUsed
)
{
  attemptsUsed = 0;

  for (
    uint8_t attempt = 1;
    attempt <= MPU_READ_MAX_ATTEMPTS;
    attempt++
  )
  {
    attemptsUsed = attempt;

    if (readMpuSampleOnce(sample))
    {
      return true;
    }

    if (attempt == 2)
    {
      recoverI2cController();
    }
    else
    {
      delayMicroseconds(MPU_READ_RETRY_DELAY_US);
    }
  }

  return false;
}

const char *gestureToReferenceLabel(int gesture)
{
  switch (gesture)
  {
    case DIR_LEFT:
      return "left";

    case DIR_RIGHT:
      return "right";

    case DIR_UP:
      return "up";

    default:
      return nullptr;
  }
}

const char *gestureToText(int gesture)
{
  switch (gesture)
  {
    case DIR_LEFT:
      return "left";

    case DIR_RIGHT:
      return "right";

    case DIR_UP:
      return "up";

    case DIR_DOWN:
      return "down";

    case DIR_NEAR:
      return "near";

    case DIR_FAR:
      return "far";

    default:
      return "none";
  }
}

void blinkLed(
  uint8_t repetitions,
  uint16_t onTimeMs,
  uint16_t offTimeMs
)
{
  for (
    uint8_t repetition = 0;
    repetition < repetitions;
    repetition++
  )
  {
    digitalWrite(LED_PIN, HIGH);
    delay(onTimeMs);

    digitalWrite(LED_PIN, LOW);
    delay(offTimeMs);
  }
}

void showPredictionWithLed(const String &gesture)
{
  if (gesture == "left")
  {
    blinkLed(1, 400, 150);
    return;
  }

  if (gesture == "up")
  {
    blinkLed(2, 250, 150);
    return;
  }

  if (gesture == "right")
  {
    blinkLed(3, 180, 150);
    return;
  }

  blinkLed(6, 80, 80);
}

void discardPendingGestures()
{
  for (
    uint8_t attempt = 0;
    attempt < 5;
    attempt++
  )
  {
    if (!apds9960.isGestureAvailable())
    {
      return;
    }

    apds9960.readGesture();
    delay(20);
  }
}

bool printAndValidatePeakToPeakValues()
{
  int16_t minimumValues[6] = {
    samples[0].ax,
    samples[0].ay,
    samples[0].az,
    samples[0].gx,
    samples[0].gy,
    samples[0].gz
  };

  int16_t maximumValues[6] = {
    samples[0].ax,
    samples[0].ay,
    samples[0].az,
    samples[0].gx,
    samples[0].gy,
    samples[0].gz
  };

  for (
    size_t sampleIndex = 1;
    sampleIndex < SAMPLE_COUNT;
    sampleIndex++
  )
  {
    const int16_t values[6] = {
      samples[sampleIndex].ax,
      samples[sampleIndex].ay,
      samples[sampleIndex].az,
      samples[sampleIndex].gx,
      samples[sampleIndex].gy,
      samples[sampleIndex].gz
    };

    for (
      uint8_t channel = 0;
      channel < 6;
      channel++
    )
    {
      if (values[channel] < minimumValues[channel])
      {
        minimumValues[channel] = values[channel];
      }

      if (values[channel] > maximumValues[channel])
      {
        maximumValues[channel] = values[channel];
      }
    }
  }

  const char *channelNames[6] = {
    "AX",
    "AY",
    "AZ",
    "GX",
    "GY",
    "GZ"
  };

  int32_t maximumAccelPeakToPeak = 0;
  int32_t maximumGyroPeakToPeak = 0;

  Serial.println("MPU6050 peak-to-peak values:");

  for (
    uint8_t channel = 0;
    channel < 6;
    channel++
  )
  {
    const int32_t peakToPeak =
      static_cast<int32_t>(maximumValues[channel]) -
      static_cast<int32_t>(minimumValues[channel]);

    Serial.printf(
      "  %s: %ld\n",
      channelNames[channel],
      static_cast<long>(peakToPeak)
    );

    if (
      channel < 3 &&
      peakToPeak > maximumAccelPeakToPeak
    )
    {
      maximumAccelPeakToPeak = peakToPeak;
    }

    if (
      channel >= 3 &&
      peakToPeak > maximumGyroPeakToPeak
    )
    {
      maximumGyroPeakToPeak = peakToPeak;
    }
  }

  const bool validMotionDetected =
    maximumAccelPeakToPeak >= MIN_ACCEL_PEAK_TO_PEAK
    || maximumGyroPeakToPeak >= MIN_GYRO_PEAK_TO_PEAK;

  Serial.printf(
    "Maximum accelerometer peak-to-peak: %ld\n",
    static_cast<long>(maximumAccelPeakToPeak)
  );

  Serial.printf(
    "Maximum gyroscope peak-to-peak: %ld\n",
    static_cast<long>(maximumGyroPeakToPeak)
  );

  Serial.printf(
    "Motion thresholds: accel >= %ld OR gyro >= %ld\n",
    static_cast<long>(MIN_ACCEL_PEAK_TO_PEAK),
    static_cast<long>(MIN_GYRO_PEAK_TO_PEAK)
  );

  Serial.print("Motion validation: ");
  Serial.println(
    validMotionDetected
      ? "ACCEPTED"
      : "REJECTED"
  );

  return validMotionDetected;
}

bool connectToWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }

  showStateScreen(
    "CONNECTING WIFI",
    "Please wait...",
    "",
    ""
  );

  Serial.print("Connecting to WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t connectionStartMs = millis();

  while (WiFi.status() != WL_CONNECTED)
  {
    if (
      millis() - connectionStartMs >=
      WIFI_CONNECTION_TIMEOUT_MS
    )
    {
      Serial.println();
      Serial.println("ERROR: WiFi connection timeout.");

      showStateScreen(
        "WIFI ERROR",
        "Connection timeout",
        "Check hotspot",
        "Retry on next capture"
      );

      return false;
    }

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected.");

  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  showStateScreen(
    "WIFI CONNECTED",
    "ESP32 IP:",
    WiFi.localIP().toString(),
    "Server: 172.20.10.7"
  );

  delay(700);

  return true;
}

String buildGestureApiUrl()
{
  String gestureUrl = API_URL;

  const int apiPathPosition =
    gestureUrl.indexOf("/api/");

  if (apiPathPosition >= 0)
  {
    gestureUrl =
      gestureUrl.substring(0, apiPathPosition) +
      "/api/gesture";
  }
  else
  {
    if (gestureUrl.endsWith("/"))
    {
      gestureUrl.remove(gestureUrl.length() - 1);
    }

    gestureUrl += "/api/gesture";
  }

  return gestureUrl;
}

bool buildInferencePayload(String &jsonBody)
{
  jsonBody = "";
  jsonBody.reserve(16000);

  jsonBody += "{\"samples\":[";

  for (
    size_t sampleIndex = 0;
    sampleIndex < SAMPLE_COUNT;
    sampleIndex++
  )
  {
    if (sampleIndex > 0)
    {
      jsonBody += ",";
    }

    jsonBody += "{\"ax\":";
    jsonBody += samples[sampleIndex].ax;

    jsonBody += ",\"ay\":";
    jsonBody += samples[sampleIndex].ay;

    jsonBody += ",\"az\":";
    jsonBody += samples[sampleIndex].az;

    jsonBody += ",\"gx\":";
    jsonBody += samples[sampleIndex].gx;

    jsonBody += ",\"gy\":";
    jsonBody += samples[sampleIndex].gy;

    jsonBody += ",\"gz\":";
    jsonBody += samples[sampleIndex].gz;

    jsonBody += "}";
  }

  jsonBody += "]}";

  return true;
}

bool extractJsonBoolean(
  const String &jsonText,
  const char *key,
  bool &value
)
{
  String keyToken = "\"";
  keyToken += key;
  keyToken += "\":";

  int valuePosition =
    jsonText.indexOf(keyToken);

  if (valuePosition < 0)
  {
    return false;
  }

  valuePosition += keyToken.length();

  while (
    valuePosition < jsonText.length() &&
    isspace(
      static_cast<unsigned char>(
        jsonText[valuePosition]
      )
    )
  )
  {
    valuePosition++;
  }

  if (
    jsonText.substring(
      valuePosition,
      valuePosition + 4
    ) == "true"
  )
  {
    value = true;
    return true;
  }

  if (
    jsonText.substring(
      valuePosition,
      valuePosition + 5
    ) == "false"
  )
  {
    value = false;
    return true;
  }

  return false;
}

bool extractJsonString(
  const String &jsonText,
  const char *key,
  String &value
)
{
  String keyToken = "\"";
  keyToken += key;
  keyToken += "\":";

  const int keyPosition =
    jsonText.indexOf(keyToken);

  if (keyPosition < 0)
  {
    return false;
  }

  const int openingQuotePosition =
    jsonText.indexOf(
      '"',
      keyPosition + keyToken.length()
    );

  if (openingQuotePosition < 0)
  {
    return false;
  }

  const int closingQuotePosition =
    jsonText.indexOf(
      '"',
      openingQuotePosition + 1
    );

  if (closingQuotePosition < 0)
  {
    return false;
  }

  value = jsonText.substring(
    openingQuotePosition + 1,
    closingQuotePosition
  );

  return true;
}

bool extractJsonFloat(
  const String &jsonText,
  const char *key,
  float &value
)
{
  String keyToken = "\"";
  keyToken += key;
  keyToken += "\":";

  int valueStartPosition =
    jsonText.indexOf(keyToken);

  if (valueStartPosition < 0)
  {
    return false;
  }

  valueStartPosition += keyToken.length();

  while (
    valueStartPosition < jsonText.length() &&
    isspace(
      static_cast<unsigned char>(
        jsonText[valueStartPosition]
      )
    )
  )
  {
    valueStartPosition++;
  }

  int valueEndPosition = valueStartPosition;

  while (valueEndPosition < jsonText.length())
  {
    const char currentCharacter =
      jsonText[valueEndPosition];

    const bool numericCharacter =
      (
        currentCharacter >= '0' &&
        currentCharacter <= '9'
      )
      || currentCharacter == '-'
      || currentCharacter == '+'
      || currentCharacter == '.'
      || currentCharacter == 'e'
      || currentCharacter == 'E';

    if (!numericCharacter)
    {
      break;
    }

    valueEndPosition++;
  }

  if (valueEndPosition <= valueStartPosition)
  {
    return false;
  }

  value = jsonText.substring(
    valueStartPosition,
    valueEndPosition
  ).toFloat();

  return true;
}

bool parseInferenceResponse(
  const String &responseBody,
  InferenceResult &result
)
{
  result.success = false;
  result.gesture = "";
  result.confidence = 0.0F;
  result.probabilityLeft = 0.0F;
  result.probabilityRight = 0.0F;
  result.probabilityUp = 0.0F;

  bool responseSuccess = false;

  if (
    !extractJsonBoolean(
      responseBody,
      "success",
      responseSuccess
    )
  )
  {
    return false;
  }

  if (!responseSuccess)
  {
    return false;
  }

  if (
    !extractJsonString(
      responseBody,
      "gesture",
      result.gesture
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "confidence",
      result.confidence
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "left",
      result.probabilityLeft
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "right",
      result.probabilityRight
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "up",
      result.probabilityUp
    )
  )
  {
    return false;
  }

  result.success = true;

  return true;
}

bool sendInferenceCapture(
  InferenceResult &result
)
{
  if (!connectToWiFi())
  {
    return false;
  }

  String jsonBody;

  if (!buildInferencePayload(jsonBody))
  {
    Serial.println(
      "ERROR: Inference JSON payload could not be built."
    );

    return false;
  }

  const String gestureApiUrl =
    buildGestureApiUrl();

  Serial.print("Gesture API URL: ");
  Serial.println(gestureApiUrl);

  Serial.print("JSON payload size: ");
  Serial.print(jsonBody.length());
  Serial.println(" bytes");

  Serial.println(
    "Sending capture to Raspberry Pi for inference..."
  );

  showStateScreen(
    "SENDING DATA",
    "Raspberry Pi",
    "100 MPU samples",
    "HTTP request"
  );

  delay(250);

  showStateScreen(
    "CNN PROCESSING",
    "NumPy 1D CNN",
    "Waiting result",
    "Please wait..."
  );

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, gestureApiUrl))
  {
    Serial.println(
      "ERROR: HTTP client initialization failed."
    );

    return false;
  }

  http.setTimeout(HTTP_REQUEST_TIMEOUT_MS);

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  const int httpResponseCode =
    http.POST(jsonBody);

  Serial.print("HTTP response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode <= 0)
  {
    Serial.print("HTTP request failed: ");
    Serial.println(
      http.errorToString(httpResponseCode)
    );

    http.end();

    return false;
  }

  const String responseBody =
    http.getString();

  http.end();

  Serial.println("Server response:");
  Serial.println(responseBody);

  if (
    httpResponseCode < 200 ||
    httpResponseCode >= 300
  )
  {
    Serial.println(
      "ERROR: Raspberry Pi rejected the inference request."
    );

    return false;
  }

  if (!parseInferenceResponse(responseBody, result))
  {
    Serial.println(
      "ERROR: Raspberry Pi response could not be parsed."
    );

    return false;
  }

  return true;
}

int readReferenceGesture()
{
  const uint32_t waitStartMs = millis();

  while (
    millis() - waitStartMs <
    GESTURE_WAIT_TIMEOUT_MS
  )
  {
    if (apds9960.isGestureAvailable())
    {
      return apds9960.readGesture();
    }

    delay(10);
  }

  return DIR_NONE;
}

void printInferenceResult(
  const InferenceResult &result,
  const char *referenceLabel
)
{
  Serial.println();
  Serial.println("CNN INFERENCE RESULT");

  Serial.print("Predicted gesture: ");
  Serial.println(result.gesture);

  Serial.printf(
    "Confidence: %.4f\n",
    result.confidence
  );

  Serial.println("Probabilities:");

  Serial.printf(
    "  left: %.4f\n",
    result.probabilityLeft
  );

  Serial.printf(
    "  right: %.4f\n",
    result.probabilityRight
  );

  Serial.printf(
    "  up: %.4f\n",
    result.probabilityUp
  );

  Serial.print("APDS reference comparison: ");

  if (referenceLabel == nullptr)
  {
    Serial.println("unavailable");
    return;
  }

  if (result.gesture == referenceLabel)
  {
    Serial.println("MATCH");
  }
  else
  {
    Serial.println("MISMATCH");
  }
}

void runCapture()
{
  Serial.println();
  Serial.println("Prepare the gesture.");

  showPrepareCountdown();

  // Do not call readGesture() before capture. The SparkFun APDS-9960
  // routine can block while waiting for a gesture sequence to finish.
  Serial.println("CAPTURE START");

  showStateScreen(
    "CAPTURING",
    "MPU6050 motion",
    "100 samples / 2 s",
    "Keep moving"
  );

  digitalWrite(LED_PIN, HIGH);

  const uint32_t captureStartUs = micros();
  uint32_t nextSampleTimeUs = captureStartUs;
  uint16_t recoveredMpuReadCount = 0;
  uint8_t maximumMpuReadAttempts = 1;

  for (
    size_t sampleIndex = 0;
    sampleIndex < SAMPLE_COUNT;
    sampleIndex++
  )
  {
    while (
      static_cast<int32_t>(
        micros() - nextSampleTimeUs
      ) < 0
    )
    {
      delayMicroseconds(100);
    }

    uint8_t attemptsUsed = 0;

    if (!readMpuSample(samples[sampleIndex], attemptsUsed))
    {
      digitalWrite(LED_PIN, LOW);

      Serial.printf(
        "ERROR: MPU6050 read failed at sample %u after %u attempts.\n",
        static_cast<unsigned int>(sampleIndex),
        static_cast<unsigned int>(attemptsUsed)
      );

      showStateScreen(
        "SENSOR ERROR",
        "MPU6050 read",
        "Capture stopped",
        "Press to retry"
      );

      blinkLed(6, 80, 80);
      delay(MESSAGE_SCREEN_TIME_MS);
      showReadyScreen();

      return;
    }

    if (attemptsUsed > 1)
    {
      recoveredMpuReadCount++;

      if (attemptsUsed > maximumMpuReadAttempts)
      {
        maximumMpuReadAttempts = attemptsUsed;
      }
    }

    nextSampleTimeUs += SAMPLE_INTERVAL_US;
  }

  const uint32_t captureDurationUs =
    micros() - captureStartUs;

  digitalWrite(LED_PIN, LOW);

  Serial.println("CAPTURE COMPLETED");

  showStateScreen(
    "CAPTURE COMPLETE",
    "Validating motion",
    "Reading reference",
    "Please wait..."
  );

  const int detectedGesture =
    readReferenceGesture();

  const char *detectedGestureText =
    gestureToText(detectedGesture);

  const char *referenceLabel =
    gestureToReferenceLabel(detectedGesture);

  Serial.printf(
    "Detected APDS-9960 reference gesture: %s\n",
    detectedGestureText
  );

  Serial.printf(
    "Captured samples: %u\n",
    static_cast<unsigned int>(SAMPLE_COUNT)
  );

  Serial.printf(
    "Capture duration: %.1f ms\n",
    captureDurationUs / 1000.0
  );

  Serial.printf(
    "Recovered MPU6050 reads: %u (maximum attempts used: %u)\n",
    static_cast<unsigned int>(recoveredMpuReadCount),
    static_cast<unsigned int>(maximumMpuReadAttempts)
  );

  const bool validMpuMotion =
    printAndValidatePeakToPeakValues();

  if (!validMpuMotion)
  {
    Serial.println();
    Serial.println(
      "CAPTURE REJECTED: MPU6050 motion was below the minimum thresholds."
    );

    Serial.println(
      "Nothing was sent to the Raspberry Pi."
    );

    showStateScreen(
      "MOTION REJECTED",
      "Movement too low",
      "Move the sensor",
      "Press to retry"
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Move the MPU6050 clearly and press the button to try again."
    );

    delay(MESSAGE_SCREEN_TIME_MS);
    showReadyScreen();

    return;
  }

  if (referenceLabel == nullptr)
  {
    Serial.println();
    Serial.println(
      "APDS reference is unavailable, but CNN inference will continue."
    );
  }
  else
  {
    Serial.println();
    Serial.printf(
      "APDS reference label: %s\n",
      referenceLabel
    );
  }

  InferenceResult inferenceResult;

  if (!sendInferenceCapture(inferenceResult))
  {
    Serial.println();
    Serial.println(
      "INFERENCE FAILED: Communication with the Raspberry Pi failed."
    );

    showStateScreen(
      "SERVER ERROR",
      "Inference failed",
      "Check WiFi / Pi",
      "Press to retry"
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Press the button to try another capture."
    );

    delay(MESSAGE_SCREEN_TIME_MS);
    showReadyScreen();

    return;
  }

  printInferenceResult(
    inferenceResult,
    referenceLabel
  );

  showStateScreen(
    "ACTUATING",
    String("Gesture: ") +
      inferenceResult.gesture,
    "Moving servo",
    "Updating outputs"
  );

  const int servoAngle =
    moveServoForGesture(
      inferenceResult.gesture
    );

  showResultSummary(
    inferenceResult,
    servoAngle
  );

  showPredictionWithLed(
    inferenceResult.gesture
  );

  delay(RESULT_SCREEN_TIME_MS);

  showProbabilityScreen(
    inferenceResult
  );

  delay(PROBABILITY_SCREEN_TIME_MS);

  Serial.println();
  Serial.println(
    "INFERENCE COMPLETED SUCCESSFULLY."
  );

  Serial.println();
  Serial.println(
    "Press the button for another capture."
  );

  showReadyScreen();
}

void setup()
{
  Serial.begin(115200);
  delay(1500);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  displayAvailable =
    display.begin(
      SSD1306_SWITCHCAPVCC
    );

  if (!displayAvailable)
  {
    Serial.println(
      "WARNING: OLED initialization failed. Core system will continue."
    );
  }
  else
  {
    display.clearDisplay();
    display.display();

    showStateScreen(
      "EMBEDDED GESTURE",
      "Booting system",
      "OLED interface: OK",
      "Please wait..."
    );
  }

  ESP32PWM::allocateTimer(0);
  gestureServo.setPeriodHertz(50);
  gestureServo.attach(
    SERVO_PIN,
    544,
    2400
  );

  servoAvailable =
    gestureServo.attached();

  if (!servoAvailable)
  {
    Serial.println(
      "WARNING: Servo attachment failed. Core system will continue."
    );
  }
  else
  {
    gestureServo.write(
      SERVO_INITIAL_ANGLE
    );

    currentServoAngle =
      SERVO_INITIAL_ANGLE;

    delay(350);
  }

  showStateScreen(
    "INITIALIZING",
    "MPU6050 + APDS",
    servoAvailable
      ? "Servo: OK"
      : "Servo: unavailable",
    "Please wait..."
  );

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);

  Serial.println();
  Serial.println(
    "WiFi gesture inference system"
  );

  if (!initializeMpu6050())
  {
    Serial.println(
      "ERROR: MPU6050 initialization failed."
    );

    showStateScreen(
      "SENSOR ERROR",
      "MPU6050 failed",
      "Check wiring",
      "Restart system"
    );

    while (true)
    {
      delay(1000);
    }
  }

  showStateScreen(
    "INITIALIZING",
    "MPU6050: OK",
    "APDS9960...",
    "Please wait..."
  );

  if (!apds9960.init())
  {
    Serial.println(
      "ERROR: APDS-9960 initialization failed."
    );

    showStateScreen(
      "SENSOR ERROR",
      "APDS9960 failed",
      "Check wiring",
      "Restart system"
    );

    while (true)
    {
      delay(1000);
    }
  }

  Wire.setClock(I2C_CLOCK_HZ);

  if (!apds9960.enableGestureSensor(false))
  {
    Serial.println(
      "ERROR: APDS-9960 gesture engine failed."
    );

    showStateScreen(
      "SENSOR ERROR",
      "APDS gesture failed",
      "Check library",
      "Restart system"
    );

    while (true)
    {
      delay(1000);
    }
  }

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );

  Serial.println("Both sensors initialized.");

  showStateScreen(
    "SENSORS READY",
    "MPU6050: OK",
    "APDS9960: OK",
    "Connecting WiFi"
  );

  delay(500);

  if (!connectToWiFi())
  {
    Serial.println(
      "WiFi is not available yet."
    );

    Serial.println(
      "The ESP32 will retry before sending a capture."
    );

    delay(MESSAGE_SCREEN_TIME_MS);
  }

  Serial.println(
    "Press the button to start an inference capture."
  );

  showReadyScreen();
}

void loop()
{
  if (!captureRequested)
  {
    delay(5);
    return;
  }

  noInterrupts();
  captureRequested = false;
  interrupts();

  if (
    millis() - lastCaptureMs <
    BUTTON_DEBOUNCE_MS
  )
  {
    return;
  }

  // Ignore button bounce and electrical noise during the complete capture,
  // HTTP request, OLED update, and servo movement.
  detachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN)
  );

  runCapture();

  // Rearm only after the button is physically released and stable.
  while (digitalRead(BUTTON_PIN) == LOW)
  {
    delay(5);
  }

  delay(BUTTON_RELEASE_STABLE_MS);

  noInterrupts();
  captureRequested = false;
  interrupts();

  lastCaptureMs = millis();

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );
}
Library
/
Embedded Systems
/
gesture_inference_wifi_oled_servo_v2.ino


#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include <SparkFun_APDS9960.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#include "secrets.h"

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint32_t I2C_CLOCK_HZ = 100000;

constexpr uint8_t MPU6050_ADDRESS = 0x68;
constexpr uint8_t MPU6050_SIGNAL_PATH_RESET = 0x68;
constexpr uint8_t MPU6050_PWR_MGMT_1 = 0x6B;
constexpr uint8_t MPU6050_PWR_MGMT_2 = 0x6C;
constexpr uint8_t MPU6050_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t MPU6050_WHO_AM_I = 0x75;

constexpr uint8_t BUTTON_PIN = 27;
constexpr uint8_t LED_PIN = 26;

constexpr int16_t SCREEN_WIDTH = 128;
constexpr int16_t SCREEN_HEIGHT = 64;

constexpr int8_t OLED_MOSI_PIN = 23;
constexpr int8_t OLED_CLK_PIN = 18;
constexpr int8_t OLED_DC_PIN = 2;
constexpr int8_t OLED_CS_PIN = 5;
constexpr int8_t OLED_RESET_PIN = 4;

constexpr uint8_t SERVO_PIN = 13;
constexpr int SERVO_RIGHT_ANGLE = 0;
constexpr int SERVO_UP_ANGLE = 90;
constexpr int SERVO_LEFT_ANGLE = 180;
constexpr int SERVO_INITIAL_ANGLE = SERVO_UP_ANGLE;
constexpr uint16_t SERVO_STEP_DELAY_MS = 8;

constexpr uint32_t RESULT_SCREEN_TIME_MS = 2200;
constexpr uint32_t PROBABILITY_SCREEN_TIME_MS = 2200;
constexpr uint32_t MESSAGE_SCREEN_TIME_MS = 1800;

constexpr size_t SAMPLE_COUNT = 100;
constexpr uint32_t SAMPLE_INTERVAL_US = 20000;

constexpr uint32_t BUTTON_DEBOUNCE_MS = 350;
constexpr uint32_t BUTTON_RELEASE_STABLE_MS = 80;
constexpr uint8_t MPU_READ_MAX_ATTEMPTS = 4;
constexpr uint32_t MPU_READ_RETRY_DELAY_US = 500;
constexpr uint32_t WIFI_CONNECTION_TIMEOUT_MS = 20000;
constexpr uint32_t HTTP_REQUEST_TIMEOUT_MS = 20000;
constexpr uint32_t GESTURE_WAIT_TIMEOUT_MS = 1500;

constexpr int32_t MIN_ACCEL_PEAK_TO_PEAK = 1500;
constexpr int32_t MIN_GYRO_PEAK_TO_PEAK = 500;

struct MotionSample
{
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;
};

struct InferenceResult
{
  bool success;
  String gesture;
  float confidence;
  float probabilityLeft;
  float probabilityRight;
  float probabilityUp;
};

SparkFun_APDS9960 apds9960;

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  OLED_MOSI_PIN,
  OLED_CLK_PIN,
  OLED_DC_PIN,
  OLED_RESET_PIN,
  OLED_CS_PIN
);

Servo gestureServo;

bool displayAvailable = false;
bool servoAvailable = false;
int currentServoAngle = SERVO_INITIAL_ANGLE;

MotionSample samples[SAMPLE_COUNT];

volatile bool captureRequested = false;

uint32_t lastCaptureMs = 0;

void IRAM_ATTR handleButtonInterrupt()
{
  captureRequested = true;
}

void drawCenteredText(
  const String &text,
  int16_t y,
  uint8_t textSize
)
{
  if (!displayAvailable)
  {
    return;
  }

  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t width = 0;
  uint16_t height = 0;

  display.setTextSize(textSize);
  display.getTextBounds(
    text,
    0,
    y,
    &x1,
    &y1,
    &width,
    &height
  );

  int16_t x =
    (
      SCREEN_WIDTH -
      static_cast<int16_t>(width)
    ) / 2;

  if (x < 0)
  {
    x = 0;
  }

  display.setCursor(x, y);
  display.print(text);
}

void showStateScreen(
  const String &title,
  const String &line1 = "",
  const String &line2 = "",
  const String &line3 = ""
)
{
  if (!displayAvailable)
  {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  drawCenteredText(title, 2, 1);
  display.drawFastHLine(
    0,
    13,
    SCREEN_WIDTH,
    SSD1306_WHITE
  );

  drawCenteredText(line1, 20, 1);
  drawCenteredText(line2, 34, 1);
  drawCenteredText(line3, 48, 1);

  display.display();
}

void showReadyScreen()
{
  const String wifiLine =
    WiFi.status() == WL_CONNECTED
      ? "WiFi: connected"
      : "WiFi: retry later";

  showStateScreen(
    "SYSTEM READY",
    "Press the button",
    "to capture gesture",
    wifiLine
  );
}

void showPrepareCountdown()
{
  for (int countdown = 3; countdown >= 1; countdown--)
  {
    showStateScreen(
      "PREPARE GESTURE",
      "Starting in",
      String(countdown),
      "Move during capture"
    );

    digitalWrite(LED_PIN, HIGH);
    delay(150);

    digitalWrite(LED_PIN, LOW);
    delay(150);
  }

  delay(100);
}

void showResultSummary(
  const InferenceResult &result,
  int servoAngle
)
{
  if (!displayAvailable)
  {
    return;
  }

  String gestureText = result.gesture;
  gestureText.toUpperCase();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  drawCenteredText("GESTURE RESULT", 2, 1);
  display.drawFastHLine(
    0,
    13,
    SCREEN_WIDTH,
    SSD1306_WHITE
  );

  drawCenteredText(gestureText, 18, 2);

  drawCenteredText(
    String("Confidence: ") +
    String(result.confidence * 100.0F, 2) +
    "%",
    41,
    1
  );

  const String servoLine =
    servoAngle >= 0
      ? String("Servo: ") +
        String(servoAngle) +
        " deg"
      : String("Servo: no command");

  drawCenteredText(servoLine, 53, 1);

  display.display();
}

void showProbabilityScreen(
  const InferenceResult &result
)
{
  showStateScreen(
    "CNN PROBABILITIES",
    String("Left:  ") +
      String(result.probabilityLeft * 100.0F, 2) +
      "%",
    String("Right: ") +
      String(result.probabilityRight * 100.0F, 2) +
      "%",
    String("Up:    ") +
      String(result.probabilityUp * 100.0F, 2) +
      "%"
  );
}

int gestureToServoAngle(const String &gesture)
{
  if (gesture == "right")
  {
    return SERVO_RIGHT_ANGLE;
  }

  if (gesture == "up")
  {
    return SERVO_UP_ANGLE;
  }

  if (gesture == "left")
  {
    return SERVO_LEFT_ANGLE;
  }

  return -1;
}

int moveServoForGesture(const String &gesture)
{
  const int targetAngle =
    gestureToServoAngle(gesture);

  if (
    !servoAvailable ||
    targetAngle < 0
  )
  {
    return -1;
  }

  const int step =
    targetAngle >= currentServoAngle
      ? 1
      : -1;

  int angle = currentServoAngle;

  while (angle != targetAngle)
  {
    gestureServo.write(angle);
    angle += step;
    delay(SERVO_STEP_DELAY_MS);
  }

  gestureServo.write(targetAngle);
  currentServoAngle = targetAngle;

  return targetAngle;
}

bool writeMpuRegister(
  uint8_t registerAddress,
  uint8_t value
)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}

bool readMpuRegister(
  uint8_t registerAddress,
  uint8_t &value
)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(registerAddress);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  if (
    Wire.requestFrom(
      MPU6050_ADDRESS,
      static_cast<uint8_t>(1)
    ) != 1
  )
  {
    return false;
  }

  value = Wire.read();

  return true;
}

bool initializeMpu6050()
{
  if (!writeMpuRegister(MPU6050_PWR_MGMT_1, 0x80))
  {
    return false;
  }

  delay(100);

  if (
    !writeMpuRegister(
      MPU6050_SIGNAL_PATH_RESET,
      0x07
    )
  )
  {
    return false;
  }

  delay(100);

  if (!writeMpuRegister(MPU6050_PWR_MGMT_1, 0x01))
  {
    return false;
  }

  delay(20);

  if (!writeMpuRegister(MPU6050_PWR_MGMT_2, 0x00))
  {
    return false;
  }

  delay(20);

  uint8_t deviceId = 0;

  if (!readMpuRegister(MPU6050_WHO_AM_I, deviceId))
  {
    return false;
  }

  Serial.printf(
    "MPU6050 device ID: 0x%02X\n",
    deviceId
  );

  return deviceId != 0x00 && deviceId != 0xFF;
}

bool readMpuSampleOnce(MotionSample &sample)
{
  while (Wire.available())
  {
    Wire.read();
  }

  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(MPU6050_ACCEL_XOUT_H);

  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  const size_t receivedBytes =
    Wire.requestFrom(
      MPU6050_ADDRESS,
      static_cast<uint8_t>(14)
    );

  if (receivedBytes != 14 || Wire.available() < 14)
  {
    while (Wire.available())
    {
      Wire.read();
    }

    return false;
  }

  sample.ax =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.ay =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.az =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  Wire.read();
  Wire.read();

  sample.gx =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.gy =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  sample.gz =
    static_cast<int16_t>(
      (Wire.read() << 8) | Wire.read()
    );

  return true;
}

void recoverI2cController()
{
  while (Wire.available())
  {
    Wire.read();
  }

  Wire.end();
  delay(2);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);
}

bool readMpuSample(
  MotionSample &sample,
  uint8_t &attemptsUsed
)
{
  attemptsUsed = 0;

  for (
    uint8_t attempt = 1;
    attempt <= MPU_READ_MAX_ATTEMPTS;
    attempt++
  )
  {
    attemptsUsed = attempt;

    if (readMpuSampleOnce(sample))
    {
      return true;
    }

    if (attempt == 2)
    {
      recoverI2cController();
    }
    else
    {
      delayMicroseconds(MPU_READ_RETRY_DELAY_US);
    }
  }

  return false;
}

const char *gestureToReferenceLabel(int gesture)
{
  switch (gesture)
  {
    case DIR_LEFT:
      return "left";

    case DIR_RIGHT:
      return "right";

    case DIR_UP:
      return "up";

    default:
      return nullptr;
  }
}

const char *gestureToText(int gesture)
{
  switch (gesture)
  {
    case DIR_LEFT:
      return "left";

    case DIR_RIGHT:
      return "right";

    case DIR_UP:
      return "up";

    case DIR_DOWN:
      return "down";

    case DIR_NEAR:
      return "near";

    case DIR_FAR:
      return "far";

    default:
      return "none";
  }
}

void blinkLed(
  uint8_t repetitions,
  uint16_t onTimeMs,
  uint16_t offTimeMs
)
{
  for (
    uint8_t repetition = 0;
    repetition < repetitions;
    repetition++
  )
  {
    digitalWrite(LED_PIN, HIGH);
    delay(onTimeMs);

    digitalWrite(LED_PIN, LOW);
    delay(offTimeMs);
  }
}

void showPredictionWithLed(const String &gesture)
{
  if (gesture == "left")
  {
    blinkLed(1, 400, 150);
    return;
  }

  if (gesture == "up")
  {
    blinkLed(2, 250, 150);
    return;
  }

  if (gesture == "right")
  {
    blinkLed(3, 180, 150);
    return;
  }

  blinkLed(6, 80, 80);
}

void discardPendingGestures()
{
  for (
    uint8_t attempt = 0;
    attempt < 5;
    attempt++
  )
  {
    if (!apds9960.isGestureAvailable())
    {
      return;
    }

    apds9960.readGesture();
    delay(20);
  }
}

bool printAndValidatePeakToPeakValues()
{
  int16_t minimumValues[6] = {
    samples[0].ax,
    samples[0].ay,
    samples[0].az,
    samples[0].gx,
    samples[0].gy,
    samples[0].gz
  };

  int16_t maximumValues[6] = {
    samples[0].ax,
    samples[0].ay,
    samples[0].az,
    samples[0].gx,
    samples[0].gy,
    samples[0].gz
  };

  for (
    size_t sampleIndex = 1;
    sampleIndex < SAMPLE_COUNT;
    sampleIndex++
  )
  {
    const int16_t values[6] = {
      samples[sampleIndex].ax,
      samples[sampleIndex].ay,
      samples[sampleIndex].az,
      samples[sampleIndex].gx,
      samples[sampleIndex].gy,
      samples[sampleIndex].gz
    };

    for (
      uint8_t channel = 0;
      channel < 6;
      channel++
    )
    {
      if (values[channel] < minimumValues[channel])
      {
        minimumValues[channel] = values[channel];
      }

      if (values[channel] > maximumValues[channel])
      {
        maximumValues[channel] = values[channel];
      }
    }
  }

  const char *channelNames[6] = {
    "AX",
    "AY",
    "AZ",
    "GX",
    "GY",
    "GZ"
  };

  int32_t maximumAccelPeakToPeak = 0;
  int32_t maximumGyroPeakToPeak = 0;

  Serial.println("MPU6050 peak-to-peak values:");

  for (
    uint8_t channel = 0;
    channel < 6;
    channel++
  )
  {
    const int32_t peakToPeak =
      static_cast<int32_t>(maximumValues[channel]) -
      static_cast<int32_t>(minimumValues[channel]);

    Serial.printf(
      "  %s: %ld\n",
      channelNames[channel],
      static_cast<long>(peakToPeak)
    );

    if (
      channel < 3 &&
      peakToPeak > maximumAccelPeakToPeak
    )
    {
      maximumAccelPeakToPeak = peakToPeak;
    }

    if (
      channel >= 3 &&
      peakToPeak > maximumGyroPeakToPeak
    )
    {
      maximumGyroPeakToPeak = peakToPeak;
    }
  }

  const bool validMotionDetected =
    maximumAccelPeakToPeak >= MIN_ACCEL_PEAK_TO_PEAK
    || maximumGyroPeakToPeak >= MIN_GYRO_PEAK_TO_PEAK;

  Serial.printf(
    "Maximum accelerometer peak-to-peak: %ld\n",
    static_cast<long>(maximumAccelPeakToPeak)
  );

  Serial.printf(
    "Maximum gyroscope peak-to-peak: %ld\n",
    static_cast<long>(maximumGyroPeakToPeak)
  );

  Serial.printf(
    "Motion thresholds: accel >= %ld OR gyro >= %ld\n",
    static_cast<long>(MIN_ACCEL_PEAK_TO_PEAK),
    static_cast<long>(MIN_GYRO_PEAK_TO_PEAK)
  );

  Serial.print("Motion validation: ");
  Serial.println(
    validMotionDetected
      ? "ACCEPTED"
      : "REJECTED"
  );

  return validMotionDetected;
}

bool connectToWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }

  showStateScreen(
    "CONNECTING WIFI",
    "Please wait...",
    "",
    ""
  );

  Serial.print("Connecting to WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t connectionStartMs = millis();

  while (WiFi.status() != WL_CONNECTED)
  {
    if (
      millis() - connectionStartMs >=
      WIFI_CONNECTION_TIMEOUT_MS
    )
    {
      Serial.println();
      Serial.println("ERROR: WiFi connection timeout.");

      showStateScreen(
        "WIFI ERROR",
        "Connection timeout",
        "Check hotspot",
        "Retry on next capture"
      );

      return false;
    }

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected.");

  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  showStateScreen(
    "WIFI CONNECTED",
    "ESP32 IP:",
    WiFi.localIP().toString(),
    "Server: 172.20.10.7"
  );

  delay(700);

  return true;
}

String buildGestureApiUrl()
{
  String gestureUrl = API_URL;

  const int apiPathPosition =
    gestureUrl.indexOf("/api/");

  if (apiPathPosition >= 0)
  {
    gestureUrl =
      gestureUrl.substring(0, apiPathPosition) +
      "/api/gesture";
  }
  else
  {
    if (gestureUrl.endsWith("/"))
    {
      gestureUrl.remove(gestureUrl.length() - 1);
    }

    gestureUrl += "/api/gesture";
  }

  return gestureUrl;
}

bool buildInferencePayload(String &jsonBody)
{
  jsonBody = "";
  jsonBody.reserve(16000);

  jsonBody += "{\"samples\":[";

  for (
    size_t sampleIndex = 0;
    sampleIndex < SAMPLE_COUNT;
    sampleIndex++
  )
  {
    if (sampleIndex > 0)
    {
      jsonBody += ",";
    }

    jsonBody += "{\"ax\":";
    jsonBody += samples[sampleIndex].ax;

    jsonBody += ",\"ay\":";
    jsonBody += samples[sampleIndex].ay;

    jsonBody += ",\"az\":";
    jsonBody += samples[sampleIndex].az;

    jsonBody += ",\"gx\":";
    jsonBody += samples[sampleIndex].gx;

    jsonBody += ",\"gy\":";
    jsonBody += samples[sampleIndex].gy;

    jsonBody += ",\"gz\":";
    jsonBody += samples[sampleIndex].gz;

    jsonBody += "}";
  }

  jsonBody += "]}";

  return true;
}

bool extractJsonBoolean(
  const String &jsonText,
  const char *key,
  bool &value
)
{
  String keyToken = "\"";
  keyToken += key;
  keyToken += "\":";

  int valuePosition =
    jsonText.indexOf(keyToken);

  if (valuePosition < 0)
  {
    return false;
  }

  valuePosition += keyToken.length();

  while (
    valuePosition < jsonText.length() &&
    isspace(
      static_cast<unsigned char>(
        jsonText[valuePosition]
      )
    )
  )
  {
    valuePosition++;
  }

  if (
    jsonText.substring(
      valuePosition,
      valuePosition + 4
    ) == "true"
  )
  {
    value = true;
    return true;
  }

  if (
    jsonText.substring(
      valuePosition,
      valuePosition + 5
    ) == "false"
  )
  {
    value = false;
    return true;
  }

  return false;
}

bool extractJsonString(
  const String &jsonText,
  const char *key,
  String &value
)
{
  String keyToken = "\"";
  keyToken += key;
  keyToken += "\":";

  const int keyPosition =
    jsonText.indexOf(keyToken);

  if (keyPosition < 0)
  {
    return false;
  }

  const int openingQuotePosition =
    jsonText.indexOf(
      '"',
      keyPosition + keyToken.length()
    );

  if (openingQuotePosition < 0)
  {
    return false;
  }

  const int closingQuotePosition =
    jsonText.indexOf(
      '"',
      openingQuotePosition + 1
    );

  if (closingQuotePosition < 0)
  {
    return false;
  }

  value = jsonText.substring(
    openingQuotePosition + 1,
    closingQuotePosition
  );

  return true;
}

bool extractJsonFloat(
  const String &jsonText,
  const char *key,
  float &value
)
{
  String keyToken = "\"";
  keyToken += key;
  keyToken += "\":";

  int valueStartPosition =
    jsonText.indexOf(keyToken);

  if (valueStartPosition < 0)
  {
    return false;
  }

  valueStartPosition += keyToken.length();

  while (
    valueStartPosition < jsonText.length() &&
    isspace(
      static_cast<unsigned char>(
        jsonText[valueStartPosition]
      )
    )
  )
  {
    valueStartPosition++;
  }

  int valueEndPosition = valueStartPosition;

  while (valueEndPosition < jsonText.length())
  {
    const char currentCharacter =
      jsonText[valueEndPosition];

    const bool numericCharacter =
      (
        currentCharacter >= '0' &&
        currentCharacter <= '9'
      )
      || currentCharacter == '-'
      || currentCharacter == '+'
      || currentCharacter == '.'
      || currentCharacter == 'e'
      || currentCharacter == 'E';

    if (!numericCharacter)
    {
      break;
    }

    valueEndPosition++;
  }

  if (valueEndPosition <= valueStartPosition)
  {
    return false;
  }

  value = jsonText.substring(
    valueStartPosition,
    valueEndPosition
  ).toFloat();

  return true;
}

bool parseInferenceResponse(
  const String &responseBody,
  InferenceResult &result
)
{
  result.success = false;
  result.gesture = "";
  result.confidence = 0.0F;
  result.probabilityLeft = 0.0F;
  result.probabilityRight = 0.0F;
  result.probabilityUp = 0.0F;

  bool responseSuccess = false;

  if (
    !extractJsonBoolean(
      responseBody,
      "success",
      responseSuccess
    )
  )
  {
    return false;
  }

  if (!responseSuccess)
  {
    return false;
  }

  if (
    !extractJsonString(
      responseBody,
      "gesture",
      result.gesture
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "confidence",
      result.confidence
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "left",
      result.probabilityLeft
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "right",
      result.probabilityRight
    )
  )
  {
    return false;
  }

  if (
    !extractJsonFloat(
      responseBody,
      "up",
      result.probabilityUp
    )
  )
  {
    return false;
  }

  result.success = true;

  return true;
}

bool sendInferenceCapture(
  InferenceResult &result
)
{
  if (!connectToWiFi())
  {
    return false;
  }

  String jsonBody;

  if (!buildInferencePayload(jsonBody))
  {
    Serial.println(
      "ERROR: Inference JSON payload could not be built."
    );

    return false;
  }

  const String gestureApiUrl =
    buildGestureApiUrl();

  Serial.print("Gesture API URL: ");
  Serial.println(gestureApiUrl);

  Serial.print("JSON payload size: ");
  Serial.print(jsonBody.length());
  Serial.println(" bytes");

  Serial.println(
    "Sending capture to Raspberry Pi for inference..."
  );

  showStateScreen(
    "SENDING DATA",
    "Raspberry Pi",
    "100 MPU samples",
    "HTTP request"
  );

  delay(250);

  showStateScreen(
    "CNN PROCESSING",
    "NumPy 1D CNN",
    "Waiting result",
    "Please wait..."
  );

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, gestureApiUrl))
  {
    Serial.println(
      "ERROR: HTTP client initialization failed."
    );

    return false;
  }

  http.setTimeout(HTTP_REQUEST_TIMEOUT_MS);

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  const int httpResponseCode =
    http.POST(jsonBody);

  Serial.print("HTTP response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode <= 0)
  {
    Serial.print("HTTP request failed: ");
    Serial.println(
      http.errorToString(httpResponseCode)
    );

    http.end();

    return false;
  }

  const String responseBody =
    http.getString();

  http.end();

  Serial.println("Server response:");
  Serial.println(responseBody);

  if (
    httpResponseCode < 200 ||
    httpResponseCode >= 300
  )
  {
    Serial.println(
      "ERROR: Raspberry Pi rejected the inference request."
    );

    return false;
  }

  if (!parseInferenceResponse(responseBody, result))
  {
    Serial.println(
      "ERROR: Raspberry Pi response could not be parsed."
    );

    return false;
  }

  return true;
}

int readReferenceGesture()
{
  const uint32_t waitStartMs = millis();

  while (
    millis() - waitStartMs <
    GESTURE_WAIT_TIMEOUT_MS
  )
  {
    if (apds9960.isGestureAvailable())
    {
      return apds9960.readGesture();
    }

    delay(10);
  }

  return DIR_NONE;
}

void printInferenceResult(
  const InferenceResult &result,
  const char *referenceLabel
)
{
  Serial.println();
  Serial.println("CNN INFERENCE RESULT");

  Serial.print("Predicted gesture: ");
  Serial.println(result.gesture);

  Serial.printf(
    "Confidence: %.4f\n",
    result.confidence
  );

  Serial.println("Probabilities:");

  Serial.printf(
    "  left: %.4f\n",
    result.probabilityLeft
  );

  Serial.printf(
    "  right: %.4f\n",
    result.probabilityRight
  );

  Serial.printf(
    "  up: %.4f\n",
    result.probabilityUp
  );

  Serial.print("APDS reference comparison: ");

  if (referenceLabel == nullptr)
  {
    Serial.println("unavailable");
    return;
  }

  if (result.gesture == referenceLabel)
  {
    Serial.println("MATCH");
  }
  else
  {
    Serial.println("MISMATCH");
  }
}

void runCapture()
{
  Serial.println();
  Serial.println("Prepare the gesture.");

  showPrepareCountdown();

  // Do not call readGesture() before capture. The SparkFun APDS-9960
  // routine can block while waiting for a gesture sequence to finish.
  Serial.println("CAPTURE START");

  showStateScreen(
    "CAPTURING",
    "MPU6050 motion",
    "100 samples / 2 s",
    "Keep moving"
  );

  digitalWrite(LED_PIN, HIGH);

  const uint32_t captureStartUs = micros();
  uint32_t nextSampleTimeUs = captureStartUs;
  uint16_t recoveredMpuReadCount = 0;
  uint8_t maximumMpuReadAttempts = 1;

  for (
    size_t sampleIndex = 0;
    sampleIndex < SAMPLE_COUNT;
    sampleIndex++
  )
  {
    while (
      static_cast<int32_t>(
        micros() - nextSampleTimeUs
      ) < 0
    )
    {
      delayMicroseconds(100);
    }

    uint8_t attemptsUsed = 0;

    if (!readMpuSample(samples[sampleIndex], attemptsUsed))
    {
      digitalWrite(LED_PIN, LOW);

      Serial.printf(
        "ERROR: MPU6050 read failed at sample %u after %u attempts.\n",
        static_cast<unsigned int>(sampleIndex),
        static_cast<unsigned int>(attemptsUsed)
      );

      showStateScreen(
        "SENSOR ERROR",
        "MPU6050 read",
        "Capture stopped",
        "Press to retry"
      );

      blinkLed(6, 80, 80);
      delay(MESSAGE_SCREEN_TIME_MS);
      showReadyScreen();

      return;
    }

    if (attemptsUsed > 1)
    {
      recoveredMpuReadCount++;

      if (attemptsUsed > maximumMpuReadAttempts)
      {
        maximumMpuReadAttempts = attemptsUsed;
      }
    }

    nextSampleTimeUs += SAMPLE_INTERVAL_US;
  }

  const uint32_t captureDurationUs =
    micros() - captureStartUs;

  digitalWrite(LED_PIN, LOW);

  Serial.println("CAPTURE COMPLETED");

  showStateScreen(
    "CAPTURE COMPLETE",
    "Validating motion",
    "Reading reference",
    "Please wait..."
  );

  const int detectedGesture =
    readReferenceGesture();

  const char *detectedGestureText =
    gestureToText(detectedGesture);

  const char *referenceLabel =
    gestureToReferenceLabel(detectedGesture);

  Serial.printf(
    "Detected APDS-9960 reference gesture: %s\n",
    detectedGestureText
  );

  Serial.printf(
    "Captured samples: %u\n",
    static_cast<unsigned int>(SAMPLE_COUNT)
  );

  Serial.printf(
    "Capture duration: %.1f ms\n",
    captureDurationUs / 1000.0
  );

  Serial.printf(
    "Recovered MPU6050 reads: %u (maximum attempts used: %u)\n",
    static_cast<unsigned int>(recoveredMpuReadCount),
    static_cast<unsigned int>(maximumMpuReadAttempts)
  );

  const bool validMpuMotion =
    printAndValidatePeakToPeakValues();

  if (!validMpuMotion)
  {
    Serial.println();
    Serial.println(
      "CAPTURE REJECTED: MPU6050 motion was below the minimum thresholds."
    );

    Serial.println(
      "Nothing was sent to the Raspberry Pi."
    );

    showStateScreen(
      "MOTION REJECTED",
      "Movement too low",
      "Move the sensor",
      "Press to retry"
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Move the MPU6050 clearly and press the button to try again."
    );

    delay(MESSAGE_SCREEN_TIME_MS);
    showReadyScreen();

    return;
  }

  if (referenceLabel == nullptr)
  {
    Serial.println();
    Serial.println(
      "APDS reference is unavailable, but CNN inference will continue."
    );
  }
  else
  {
    Serial.println();
    Serial.printf(
      "APDS reference label: %s\n",
      referenceLabel
    );
  }

  InferenceResult inferenceResult;

  if (!sendInferenceCapture(inferenceResult))
  {
    Serial.println();
    Serial.println(
      "INFERENCE FAILED: Communication with the Raspberry Pi failed."
    );

    showStateScreen(
      "SERVER ERROR",
      "Inference failed",
      "Check WiFi / Pi",
      "Press to retry"
    );

    blinkLed(6, 80, 80);

    Serial.println();
    Serial.println(
      "Press the button to try another capture."
    );

    delay(MESSAGE_SCREEN_TIME_MS);
    showReadyScreen();

    return;
  }

  printInferenceResult(
    inferenceResult,
    referenceLabel
  );

  showStateScreen(
    "ACTUATING",
    String("Gesture: ") +
      inferenceResult.gesture,
    "Moving servo",
    "Updating outputs"
  );

  const int servoAngle =
    moveServoForGesture(
      inferenceResult.gesture
    );

  showResultSummary(
    inferenceResult,
    servoAngle
  );

  showPredictionWithLed(
    inferenceResult.gesture
  );

  delay(RESULT_SCREEN_TIME_MS);

  showProbabilityScreen(
    inferenceResult
  );

  delay(PROBABILITY_SCREEN_TIME_MS);

  Serial.println();
  Serial.println(
    "INFERENCE COMPLETED SUCCESSFULLY."
  );

  Serial.println();
  Serial.println(
    "Press the button for another capture."
  );

  showReadyScreen();
}

void setup()
{
  Serial.begin(115200);
  delay(1500);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  displayAvailable =
    display.begin(
      SSD1306_SWITCHCAPVCC
    );

  if (!displayAvailable)
  {
    Serial.println(
      "WARNING: OLED initialization failed. Core system will continue."
    );
  }
  else
  {
    display.clearDisplay();
    display.display();

    showStateScreen(
      "EMBEDDED GESTURE",
      "Booting system",
      "OLED interface: OK",
      "Please wait..."
    );
  }

  ESP32PWM::allocateTimer(0);
  gestureServo.setPeriodHertz(50);
  gestureServo.attach(
    SERVO_PIN,
    544,
    2400
  );

  servoAvailable =
    gestureServo.attached();

  if (!servoAvailable)
  {
    Serial.println(
      "WARNING: Servo attachment failed. Core system will continue."
    );
  }
  else
  {
    gestureServo.write(
      SERVO_INITIAL_ANGLE
    );

    currentServoAngle =
      SERVO_INITIAL_ANGLE;

    delay(350);
  }

  showStateScreen(
    "INITIALIZING",
    "MPU6050 + APDS",
    servoAvailable
      ? "Servo: OK"
      : "Servo: unavailable",
    "Please wait..."
  );

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);

  Serial.println();
  Serial.println(
    "WiFi gesture inference system"
  );

  if (!initializeMpu6050())
  {
    Serial.println(
      "ERROR: MPU6050 initialization failed."
    );

    showStateScreen(
      "SENSOR ERROR",
      "MPU6050 failed",
      "Check wiring",
      "Restart system"
    );

    while (true)
    {
      delay(1000);
    }
  }

  showStateScreen(
    "INITIALIZING",
    "MPU6050: OK",
    "APDS9960...",
    "Please wait..."
  );

  if (!apds9960.init())
  {
    Serial.println(
      "ERROR: APDS-9960 initialization failed."
    );

    showStateScreen(
      "SENSOR ERROR",
      "APDS9960 failed",
      "Check wiring",
      "Restart system"
    );

    while (true)
    {
      delay(1000);
    }
  }

  Wire.setClock(I2C_CLOCK_HZ);

  if (!apds9960.enableGestureSensor(false))
  {
    Serial.println(
      "ERROR: APDS-9960 gesture engine failed."
    );

    showStateScreen(
      "SENSOR ERROR",
      "APDS gesture failed",
      "Check library",
      "Restart system"
    );

    while (true)
    {
      delay(1000);
    }
  }

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );

  Serial.println("Both sensors initialized.");

  showStateScreen(
    "SENSORS READY",
    "MPU6050: OK",
    "APDS9960: OK",
    "Connecting WiFi"
  );

  delay(500);

  if (!connectToWiFi())
  {
    Serial.println(
      "WiFi is not available yet."
    );

    Serial.println(
      "The ESP32 will retry before sending a capture."
    );

    delay(MESSAGE_SCREEN_TIME_MS);
  }

  Serial.println(
    "Press the button to start an inference capture."
  );

  showReadyScreen();
}

void loop()
{
  if (!captureRequested)
  {
    delay(5);
    return;
  }

  noInterrupts();
  captureRequested = false;
  interrupts();

  if (
    millis() - lastCaptureMs <
    BUTTON_DEBOUNCE_MS
  )
  {
    return;
  }

  // Ignore button bounce and electrical noise during the complete capture,
  // HTTP request, OLED update, and servo movement.
  detachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN)
  );

  runCapture();

  // Rearm only after the button is physically released and stable.
  while (digitalRead(BUTTON_PIN) == LOW)
  {
    delay(5);
  }

  delay(BUTTON_RELEASE_STABLE_MS);

  noInterrupts();
  captureRequested = false;
  interrupts();

  lastCaptureMs = millis();

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );
}
