* Version 1

# Proyecto Final Electronica Digital III

###  CAT -1 ###


trabajo final para electronica basado en control de un puntero laser realimentado 
por una camara web mediante el procesamiento de imagenes.

## Getting Started

Estas instrucciones seran para que usted copie el proyecto y lo corra en su maquina local
para propositos de desarrollo y testing . 

### Prerequisites

* Python 2.7 + librerias 
* Libreria Pyserial 2.7 
* Open CV 2 para Python 2.7
* LPC 1769
* LPCxpreso
* _ReadMeFirst_LPCX176x_cmsis2 (cr_section_macros.h)

### Installing

* Intall Python 2.7 + librerias 
* Intall Libreria Pyserial 2.7 
* Intall Open CV 2 para Python 2.7
* Intall LPCXpresso IDE
* Import _ReadMeFirst_LPCX176x_cmsis2 (Ubication\nxp\LPCXpresso_8.2.2_650\lpcxpresso\Examples)


## Deployment

Un láser es reflexionado por medio de dos espejos controlados por motores PAP, uno controla su desplazamiento vertical y el otro el horizontal.
Tomando una Referencia inicial como el Centro de la imagen que cuenta con una resolucion 640x480 pixels. 
El láser de encontrarse en cualquier posición se dirige al centro. Luego por medio de una PC se le indica una nueva posición.
Al indicarle una nueva posición se calcula la trayecto que debe realizar el laser hacia la nueva posición. Este resultado se lo envía a la LPC por medio de comunicación Serie y está traduce la petición hacia los controladores indicando el paso y la dirección . Estos ponen en funcionamiento el motores.
Ante cualquier perturbación de la posición del láser, mediante el procesamiento de imagen se obtiene la posición actual, para luego calcular la diferencia con la posición de referencia, así retornar al estado de régimen

![alt pic](https://bitbucket.org/ser0090/cat_1-diii/src/504faeb983eb37c45bbfbded97260cc8edcc8d63/Imagenes/P_20171116_212049.jpg)
## Built With

* [OpenCV2](http://opencv-python-tutroals.readthedocs.io/en/latest/py_tutorials/py_setup/py_setup_in_windows/py_setup_in_windows.html) - The web framework used
* [LPCXpreso IDE](https://www.nxp.com/support/developer-resources/software-development-tools/lpc-developer-resources-/lpc-microcontroller-utilities/lpcxpresso-ide-v8.2.2:LPCXPRESSO) - Dependency Management
* [PYserila2.7](https://pypi.python.org/pypi/pyserial/2.7) - Used to generate UART

## Authors

* **Martin Barrera** - *Initial work* - [Martin Barrera](https://bitbucket.org/iotincho/)
* **Sergio Sulca** - *Initial work* - [Ser Sul](https://bitbucket.org/ser0090/)

## License


## Acknowledgments

* Nadie Por ahora 