set HOME_PATH=%CD%

set JAVA_PATH=C:\Program Files (x86)\Java\jdk1.7.0_45\bin\java.exe
set JAVA_OPTS=-Dfile.encoding=UTF-8 -classpath "%HOME_PATH%\..\bin" com.example.mlnandroid.MainActivity

cd %HOME_PATH%\..
"%JAVA_PATH%" %JAVA_OPTS%
pause
