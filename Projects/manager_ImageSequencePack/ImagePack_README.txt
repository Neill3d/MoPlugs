

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

���������� �������� � ���������� ����������. ����� ���� �� ����� ���� ����� ��� FBVideoImage � �� ��� �� FBVideoMemory.
����� ������� � ���� �������� ��� �����, �� �� ���� ��������� ��������� � ����� ������. ���������� ��� ����� ����� �������
����� ��������� ����� ���� ����� ����������. �.�. ���������� �������� ���� � ������, ������� ���������� ������ � ������� �����,
������ �� � ����� ������ ������ FBVideoMemory � ��������� � ��������.

Sergey Solohin (Neill3d) 2015