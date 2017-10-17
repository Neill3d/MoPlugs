

	== DDS Image sequences for Autodesk MotionBuilder ==

Add dds images into the zip archive without compression level. Rename file extension to *.imgPack

Plugin add support for that extension


== TODO: ==

 + use default texture (simple white) for each loaded *.imgPack
 - read information from archive (number of files, names) and set sequence properties
 - load dds file and display current frame
 - organize kind of pre-caching
 - threading, optimization, speed tests

== LOG ==

24.11.15

Непонятная ситуация с поддержкой расширений. Медиа файл на самом деле имеет тип FBVideoImage и не как не FBVideoMemory.
Таким образом я могу вытащить имя файла, но не могу управлять текстурой в видео памяти. Получается что нужен такой вариант
чтобы подменить медиа файл после добавления. Т.е. определить исходный путь к архиву, оценить содержимое архива и удалить медиа,
создав на её месте другую класса FBVideoMemory и подключив к текстуре.

Sergey Solohin (Neill3d) 2015