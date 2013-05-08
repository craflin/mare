Mare
====

Mare steht für Make Replacement und ist ein ein Werkzeug ähnlich Make zur Automatisierung des Build-Vorgangs von Softwareprojekten. Dazu werden von Mare mit Hilfe von Regeln zur Übersetzung der Dateien des Projektes Abhängigkeiten zwischen Quelldateien und Zielen verwaltet und darüber bestimmt, welche Ziele nach Änderungen an Quelldateien zur Aktualisierung der Ziele neu erstellt werden müssen. Im Gegensatz zu Make ist das von Mare benutze Dateiformat, welches die Übersetzungsregeln beinhaltet mehr an die Bedürfnisse moderner Softwareprojekte angepasst.

Motivation
----------

Make ist der Klassiker unter den Build-Tools und gehört bei Buildsystemen für UNIX-ähnliche Systeme fast immer dazu. Es ist relativ einfach, universell und lässt sich schnell und einfach einsetzen. Doch Make allein wird inzwischen selten als eigenständiges Buildsystem benutzt, da der Funktionsumfang nicht mehr den Anforderungen moderner Softwareprojekte gerecht wird. Häufig werden Makefile-Generatoren wie automake, cmake, qmake, prebuild und andere Werkzeuge eingesetzt, um dieses Problem zu umgehen.

* Unterstützt von Versionsverwaltungssystemen wie SVN oder Git nutzen viele Projekte das Dateisystem zur Verwaltung der Quelldateien. Meist werden alle Quelldateien in Verzeichnissen geordnet abgelegt, so dass jedem Verzeichnis Übersetzungsregeln für die darin enthaltenen Dateien zugeordnet werden können. Wenn Make allein genutzt wird, dann wird eine Liste aller Dateien benötigt, die ohne die Nutzung von Kommandozeilenbefehlen oder anderen Erweiterungen nicht automatisch erstellt werden kann. Meist werden, Dateilisten daher entweder manuell in einem Makefile angeben oder von einem Makefile-Generator erzeugt.

* In einem Makefile werden Übersetzungsregeln sehr universell ohne Konventionen oder zusätzlichen Informationen angegeben. Daher ist die Struktur des Projekts und der Zweck einer Übersetzungsregel schwer aus dem Makefile rekonstruierbar. Um die Struktur eines Projektes einem Entwickler aufzeigen zu können, nutzen viele Entwicklungswerkzeuge daher eigene Dateiformate zur Beschreibung eines Projekts. Dies führt dazu, dass einige projektspezifische Informationen in mehren Dateien zum Teil redundant verwaltet werden müssen, was die Pflege dieser Information erschwert.

* Die gängigen Buildsysteme wie autotools oder cmake sind darauf ausgelegt mit zum Teil sehr unterschiedlich konfigurierten Plattformen umgehen zu können. Die adäquate Spezifikation der externen Abhängigkeiten eines Projekts kann aufgrund dessen sehr Zeitaufwendig sein. Häufig werden bei Softwareprojekten jedoch festgelegt, was für Plattformen unterstützt werden und wie diese Konfiguriert sein sollen. Daher würde auch ein weniger aufwendiges Buildsystem ausreichen. Dennoch werden auch bei solchen Projekten die Makefile-Generatoren gerne eingesetzt, da sie weitere Vorteile bieten. Das genutzt Buildsystem ist daher häufig viel umfangreicher und schwieriger zu warten, als es sein müsste.

Funktionsweise
--------------

Mare ist ein kleines eigenständiges Programm, dass in einem Arbeitsverzeichnis aufgerufen wird. Im Arbeitsverzeichnis sucht es nach einer Datei namens "Marefile". Diese Datei beschreibt nach welchen Regeln die Quelldateien in Ziele übersetzt werden. Mare stellt fest, welche Ziele neu gebaut werden müssen und ruft die dafür notwendigen Tools auf. Anstatt den Buildvorgang direkt auszuführen, kann das Marefile auch eingeschränkt in Projektdateien für eine IDE (z.B. Visual Studio, CodeBlocks, CodeLite) übersetzt werden.

In einem Marefile gibt es drei spezielle Listen mit den Namen "targets", "configurations" und "platforms". "configurations" listet alle verfügbaren Konfiguration (z.B. Debug und Release) auf und "targets" enthält alle Ziele, die erstellt werden können, und die dafür benötigten Übersetzungsregeln. "platforms" listet mögliche Zielplattformen auf und wird in der Regel nur für Projekte benötigt in denen Cross-Compiler genutzt werden. Ein Marefile für eine einfache C++-Anwendung, bei der alle Quelldateien in einem Verzeichnis "src" liegen kann z.B. wie folgt aussehen:

```
configurations = { Debug, Relase }
targets = {
  myApplication = cppApplication + {
    files = {
      "src/**.cpp" = cppSource
    }
  }
}
```

Kompilation
-----------

### Windows

Um Mare unter Windows zu kompilieren, wird git und Visual Studio 2010 (oder neuer) benötigt.
https://github.com/craflin/mare.git wird in ein beliebiges Arbeitsverzeichnis geklont. Im Verzeichnis "VS2010" befinden sich dann Projektdateien für Visual Studio mit denen Mare kompiliert werden kann.

### Linux

Um Mare unter Linux (oder einem ähnlichen Betriebssystem) zu übersetzen, wird git, ein g++ kompatibler Compiler und make benötigt:

```
$ cd /your/working/directory
$ git clone https://github.com/craflin/mare.git mare
$ cd mare
$ make
```

Die ausführbare Datei befindet sich, dann im Verzeichnis "Debug". Diese kann dazu genutzt werden, um Mare auch ohne Debug-Symbole zu übersetzen:

```
$ cd /your/working/directory/mare
$ Debug/mare config=Release
```

Das Marefile
------------

Ein Marefile besteht aus verschachtelten assoziativen Listen, bei denen der Schlüssel eine Zeichenkette ist und der Datenwert eine weitere assoziative Liste sein kann. Datenwerte können aber auch weggelassen werden. Assoziative Listen können auch als Zeichenkette interpretiert werden, indem alle in der Liste enthaltenen Schlüssel mit Leerzeichen getrennt aneinander gereiht werden. Innerhalb einer assoziativen Liste können mehrere darin enthaltene assoziative Listen definiert werden, indem mehrere Schlüssel mit Leerzeichen getrennt angegeben werden. Dazu kann auch eine als Zeichenkette interpretierte assoziative Liste in einem Schlüssel eingefügt werden. Zudem können assoziative Listen aus anderen assoziativen Listen zusammengesetzt werden und diverse Fallunterscheidungen für verschiedene Konfiguration durchgeführt werden.

Ein in einem Marefile angegebenes Ziel (hier "Example1") enthält eine Menge an Eingabedateien ("input"), Ausgabedateien ("output") und einen Befehl ("command"), mit dem aus den Eingabedateien die Ausgabedateien erstellt werden können:

```
targets = {
  Example1 = {
    input = "file1.o file2.o"
    output = "Example1"
    command = "g++ file1.o file2.o -o Example1"
  }
}
```

Die Dateien aus "input" und "output" werden hier mehrfach aufgeführt. Um dies zu vermeiden kann zur Vereinfachung auch Folgendes geschrieben werden: 

```
targets = {
  Example1 = {
    input = "file1.o file2.o"
    output = "Example1"
    command = "g++ $(input) -o $(output)"
  }
}
```

Die Objektdateien ("file1.o file2.o") sollen bei diesem Beispiel zunächst aus .cpp-Dateien erstellt werden. Dazu enthält ein Ziel einen Schlüssel namens "files", welcher Regeln enthält um Quelldateien zu verarbeiten: 

```
targets = {
  Example1 = {
    input = "file1.o file2.o"
    output = "Example1"
    command = "g++ $(input) -o $(output)"
    
    cppFlags = "-O3"
    
    files = {
      "file1.cpp" = {
        input = "file1.cpp"
        output = "file1.o"
        command = "g++ $(cppFlags) -c $(input) -o $(output)"
      }
      "file2.cpp" = {
        input = "file2.cpp"
        output = "file2.o"
        command = "g++ $(cppFlags) -c $(input) -o $(output)"
      }
    }
  }
}
```

Die Regeln für die beiden Dateien "file1.cpp" und "file2.cpp" können zusammengefasst werden:

```
targets = {
  Example1 = {
    input = "file1.o file2.o"
    output = "Example1"
    command = "g++ $(input) -o $(output)"
    
    cppFlags = "-O3"
    
    files = {
      "file1.cpp file2.cpp" = {
        input = "$(file)"
        output = "$(patsubst %.cpp,%.o,$(file))"
        command = "g++ $(cppFlags) -c $(input) -o $(output)"
      }
    }
  }
}
```

Die Regel zum Übersetzten einer .cpp-Datei lässt sich auch auslagern, womit die Spezifikation überschaubarer wird: 

```
targets = {
  Example1 = {
    input = "file1.o file2.o"
    output = "Example1"
    command = "g++ $(input) -o $(output)"
    
    cppFlags = "-O3"
    
    files = {
      "file1.cpp file2.cpp" = myCppSource
    }
  }
}

myCppSource = {
  input = "$(file)"
  output = "$(patsubst %.cpp,%.o,$(file))"
  command = "g++ $(cppFlags) -c $(input) -o $(output)"
}
```

Die Liste der Objektdateien ("file1.o file2.o") kann automatisch aus der Quelldateienliste erzeugt werden und der Name der Ausgabedatei "Example1" kann aus dem Namen des Ziels abgeleitet werden:

```
targets = {
  Example1 = {
    input = "$(patsubst %.cpp,%.o,$(files))"
    output = "$(target)"
    command = "g++ $(input) -o $(output)"
    
    cppFlags = "-O3"
    
    files = {
      "file1.cpp file2.cpp" = myCppSource
    }
  }
}
...
```

Damit können auch die Regeln zum Linken des Programms ausgelagert werden:

```
targets = {
  Example1 = myCppApplication + {
    cppFlags = "-O3"
    files = {
      "file1.cpp file2.cpp" = myCppSource
    }
  }
}

myCppSource = {
  input = "$(file)"
  output = "$(patsubst %.cpp,%.o,$(file))"
  command = "g++ $(cppFlags) -c $(input) -o $(output)"
}

myCppApplication = {
  input = "$(patsubst %.cpp,%.o,$(files))"
  output = "$(target)"
  command = "g++ $(input) -o $(output)"
}
```

Die Liste der Quelldateien kann automatisch aus im Dateisystem vorhandenen Dateien generiert werden:

```
targets = {
  Example1 = myCppApplication + {
    cppFlags = "-O3"
    files = {
      "**.cpp" = myCppSource
    }
  }
}
...
```

Einzelne Schlüssel bei ausgelagerten Regeln können beim einbinden der Regeln überschrieben werden. Wenn hier der Name der Ausgabedatei z.B. nicht "Example1" sein soll, kann "output" nachträglich verändert werden:

```
targets = {
  Example1 = myCppApplication + {
    output = "Example1.blah"
    cppFlags = "-O3"
    files = {
      "**.cpp" = myCppSource
    }
  }
}
...
```

Für einfache c- und cpp-Anwendungen gibt es bereits eingebaute Regeln (cApplication, cppApplication, cDynamicLibrary, cppDynamicLibrary, cStaticLibrary, cppStaticLibrary, cSource und cppSource) mit denen das Marefile vereinfacht werden kann. (siehe Abschnitt "voreingebaute Regeln")

### Spezialisierung

Mit einer "if <expr> <statements> [else <statements>"-Anweisung können bei der Angabe einer Liste spezielle Anpassungen für verschiedene Konfigurationen vorgenommen werden:

```
cppFlags = "-mmmx"
if configuration == "Release" {
  cppFlags += "-O3"
}
...
```

oder

```
cppFlags = {
  "-mmmx"
  if configuration == "Release" { "-O3" }
}
...
```

oder

```
cppFlags = "-mmmx -O3"
if configuration != "Release" {
  cppFlags -= "-O3"
}
...
```

oder

```
cppFlags = {
  "-mmmx -O3"
  if configuration != "Release" { -"-O3" }
}
...
```

Dafür sind die Folgenden "Variablen" hilfreich:
* "configuration" - enthält den Namen der aktuellen Konfiguration ("Debug", "Release", ...) 
* "platform" - enthält den Namen der Zielplattform ("Win32", "Linux", "MacOSX", ...) 
* "host" - enthält den Namen der Hostplattform ("Win32", "Linux", "MacOSX", ...) 
* "tool" - enthält den Namen eines Übersetzers ("vcxproj", "codelite", "codeblocks", "cmake") 
* "target" - enthält den Namen des aktuell zu übersetzenden Ziels 
* "architecture" - enthält die Architektur des Hostsystems ("i686", "x86_64") 

### Include

Überall in einer Liste, wo ein Schlüssel angegeben werden kann, können auch Listen aus einer externen Datei eingebunden werden:

```
targets = {
  include "Example1.mare"
}
...
```

### Dateinamen Wildcards

Wenn Wildcards in einem Schlüssel benutzt werden, dann wird das Dateisystem nach passenden Dateien durchsucht und eine mit Leerzeichen getrennte Liste der gefunden Dateien an die Stelle des Schlüssels eingefügt. Im folgendem Beispiel wird z.B. "**.cpp" durch "file1.cpp file2.cpp" ersetzt, sofern diese beiden Dateien im Dateisystem existieren:

```
targets = {
  Example1 = myCppApplication + {
    files = {
      "**.cpp" = myCppSource
    }
  }
}
...
```

Dabei werden die folgenden Wildards verstanden:
* "*" - eine beliebige Zeichenkette in einem Dateinamen (z.B. "*.cpp" ist kompatibel mit "ab.cpp", "bcd.cpp")
* "?" - ein beliebiges Zeichen in einem Dateinamen (z.B. "a?.cpp" ist kompatibel mit "ab.cpp", "ac.cpp" aber nicht mit "aef.cpp") 
* "**" - eine beliebige Zeichenkette in einem Pfadnamen (z.B. "**.cpp" ist kompatibel mit "aa.cpp", "bb.cpp", "subdir/bbws.cpp", "subdir/subdir/bassb.cpp") 

### Leerzeichen in Schlüsseln

Leerzeichen in einem Schlüssel werden so interpretiert, dass es sich um eine Liste von Schlüssel handelt. Damit kann mehreren Schlüsseln gleichzeitig ein Datenwert zugeordnet werden. Wenn ein Schlüssel aber tatsächlich ein Leerzeichen enthalten sein soll (z.B. für einen Dateinamen mit Leerzeichen) kann die Zeichenkette in "\"" angegeben werden: 

```
myKey = "\"file name.txt\""
```

(Dieses Feature ist experimentell.)

### ',' und ';'

Angaben in einem Marefile können optional mit Komma oder Semikolon getrennt werden:

```
targets = {
  Example1 = myCppApplication + {
    files = {
      "*.cpp" = myCppSource;
    },;;,,
  },
};
...
```

### Funktionen

Innerhalb eines Schlüssels können Funktionen der Form "$(function arguments)" benutzt werden. Diese verhalten sich so, wie die Funktionen die auch in einem Gnu Makefile genutzt werden können (siehe http://www.gnu.org/software/make/manual/make.html#Functions). Jedoch sind derzeit noch nicht alle Funktionen implementiert. Implementiert sind die Folgenden:
* subst, patsubst, filter, filter-out, firstword, lastword, dir, notdir, suffix, basename, addsuffix, addprefix, if, foreach, origin 

Zusätzlich gibt es noch:
* lower - wandelt alle Zeichen der Wörter einer Liste in kleingeschriebene Buchstaben um ("$(lower AbC)" wird zu "abc")
* upper - wandelt alle Zeichen der Wörter einer Liste in großgeschriebene Buchstaben um ("$(upper dDd)" wird zu "DDD")
* readfile - fügt den Inhalt einer Datei ein (z.B: "$(readfile anyfile.d)") 

### voreingebaute Regeln

Für einfache C- und C++-Anwendungen/Dlls/Libs, gibt es so genannte voreingebaute Regeln (built-in rules), die von den Übersetzen (für z.B. Visual Studio oder CMake) unterschiedlich interpretiert werden, um eine möglichst "native" Übersetzung zu ermöglichen. 
* cppSource, cSource - Regeln für cpp/c Quelldateien 
* cppApplication, cApplication - Regel für normale cpp/c Programme
* cppDynamicLibrary, cDynamicLibrary - Regel für cpp/c DLLs bzw. "shared objects" 
* cppStaticLibrary, cStaticLibrary - Regeln für statische cpp/c Bibliotheken 

Diese Regeln können mit folgenden Listen angepasst werden:
* linker - Der benutzte Linker (Standardwert ist "gcc" bei cApplication oder cDynamicLibrary; "g++" bei cppApplication oder cppDynamicLibrary und "ar" bei cppStaticLibrary oder cStaticLibrary ) 
* linkFlags, libPaths, libs - Flags für den Linker
* cCompiler, cppCompiler - Der benutzte Compiler (Standardwert ist "gcc" bei cApplication, cDynamicLibrary oder cStaticLibrary und "g++" bei cppApplication, cppDynamicLibrary oder cppStaticLibrary)
* cFlags, cppFlags, defines, includePaths - Flags für den Compiler
* buildDir - Verzeichnis in dem Objektdateien abgelegt werden (Standardwert ist "$(configuration)") 

Ein einfaches Marefile wie

```
targets = {
  Example1 = cppApplication + {
    defines = { "NDEBUG" }
    libs = { "jpeg" }
    includePaths = { "anypath1", "anypath2" }
    files = {
      "*.cpp" = cppSource
    }
  }
}
```

sollte sowohl direkt von Mare verstanden werden, wie auch in z.B. Visual Studio Projektdateien oder in Dateien für CMake übersetzt werden können. Die Übersetzer für CodeLite, CodeBlocks, NetBeans sind bisher nicht ganz vollständig, so dass ein Marefile für diese IDEs noch nicht direkt übersetzt werden kann. Die CodeLite, CodeBlocks, NetBeans Übersetzer (und auch der Visual Studio Übersetzer) unterstützen jedoch die Verwendung eines externen Buildsystems, so dass Mare als Buildsystem in CodeLite, CodeBlocks, NetBeans oder Visual Studio genutzt werden kann. Dazu können innerhalb eines Ziels die Felder "buildCommand", "reBuildCommand" und "cleanCommand" benutzt werden:

```
targets = {
  Example1 = cppApplication + {
    defines = { "NDEBUG" }
    libs = { "jpeg" }
    includePaths = { "anypath1", "anypath2" }
    files = {
      "*.cpp" = cppSource
    }
    
    if tool = "codelite" || tool == "codeblocks" {
      buildCommand = "./mare $(target) config=$(configuration)"
      cleanCommand = "./mare clean $(target) config=$(configuration)"
      reBuildCommand = "./mare rebuild $(target) config=$(configuration)"
    }
  }
}
```

### Übersetzer

Die Übersetzer sind alle noch nicht ganz Vollständig und können daher noch nicht alle vorgesehenen Features. Hier eine Übersicht über den aktuellen Entwicklungsstand:
<table>
  <tr>
    <td></td>
    <td>mare&nbsp;1)</td>
    <td>vcxproj</td>
    <td>make</td>
    <td>codelite</td>
    <td>codeblocks</td>
    <td>cmake</td>
    <td>netbeans</td>
  </tr>
  <tr>
    <td>configurations</td>
    <td>geht</td>
    <td>geht</td>
    <td>geht</td>
    <td>geht</td>
    <td>geht</td>
    <td></td>
    <td>geht</td>
  </tr>
  <tr>
    <td>platforms</td>
    <td>geht</td>
    <td>?</td>
    <td>geht</td>
    <td>?</td>
    <td>?</td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>{c,cpp}{Source,Application,DynamicLibrary,StaticLibrary}</td>
    <td>geht</td>
    <td>geht</td>
    <td>geht</td>
    <td></td>
    <td></td>
    <td>geht</td>
    <td></td>
  </tr>
  <tr>
    <td>{c,cpp}Compiler</td>
    <td>geht</td>
    <td></td>
    <td>geht</td>
    <td></td>
    <td></td>
    <td>geht</td>
    <td></td>
  </tr>
  <tr>
    <td>linker</td>
    <td>geht</td>
    <td></td>
    <td>geht</td>
    <td></td>
    <td></td>
    <td></td>
    <td></td>
  </tr>
  <tr>
    <td>{build,clean,reBuild}Command</td>
    <td></td>
    <td>geht</td>
    <td></td>
    <td>geht</td>
    <td>geht</td>
    <td></td>
    <td>geht</td>
  </tr>
</table>

1) keine Übersetzung, direkte Interpretation des Marefiles
