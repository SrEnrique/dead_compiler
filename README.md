# dead_compiler
Compilador con Flex, Bison y llvm




##Requisitos 
```
  flex 
  bison 
  llvm 
  make
  clang++-3.5
```
Antes de compilar es necesario verificar que tenemos instalados los paquetes anteriormente mencionados , si no instálalos.
##Instalar flex, bison y llvm
###Instalar llvm
```
sudo sh -c 'echo "#LLVM 3.5 " >> /etc/apt/sources.list'
sudo sh -c 'echo "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.5 main" >> /etc/apt/sources.list'
sudo sh -c 'echo "deb-src http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.5 main " >> /etc/apt/sources.list'

sudo apt-get update

sudo apt-get install llvm-3.5
```

Ahora tenemos que verificar que en ` /usr/include/ `  se encuentre dos carpetas llamadas `llvm` y `llvm-c` , si no: 
``` 
sudo ln -s /usr/include/llvm-3.5/llvm /usr/include/llvm 
sudo ln -s /usr/include/llvm-c-3.5/llvm-c /usr/include/llvm-c 
```


Una vez instalado llvm continuamos instalando flex y bison 
###Instalar felx y bison
```
sudo apt-get install flex bison clang++-3.5
```

###build
Nos dirigimos a la carpeta del proyecto y hacemos  
``` 
make 
```

###Ejemplo 
```
variable a = 10 : entero
 		 b = 12 : entero

funcion func() : int 
inicio
	variable x = 14 : entero
	regresa x 
fin


 echo(a)
 echo(b)
 variable c : entero
 c = func()
 echo(c)
```

Si 
solucion
```
sudo apt-get install lib32z1-dev &&
sudo apt-get install libncurses5-dev
```
