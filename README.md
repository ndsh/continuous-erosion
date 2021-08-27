# Continuous Erosion
## What is the narrative of an artefact that destroys itself in the presence of the digital human?

Developed during the MAiR (Media Artist in Residence) 2017 at the residency site in Prolom Banja, a village surrounded by nature, this artefact explores the geological erosion of mythical stone formations at Đavolja Varoš (Devil’s Town) and accelerates the process by the intrusion of the digital layer that is placed upon humans in an almost ubiquitously way.

An earth surface process of erosion through friction is encapsulated within a physical artefact.
Human activities, such as intensive farming methods, are ultimately speeding up this process between 10 to 40 times its natural rate. The artefact picks up digital „pollution“ from smartphone devices in its vicinity. Human interference, illustrated as digital pollution through smartphones, speeds up the process of the artefact unwillingly.
Bombardement of invisible radio signals from our phones is not perceivable with the naked eye, but we can experience its influences physically and mentally. So does nature if we go down the chain of supply while scouting for more land and digging for resources.

<img src="https://www.julian-h.de/wp-content/uploads/2019/11/180603-continuous-erosion-schematic-01.png">
<img src="https://www.julian-h.de/wp-content/uploads/2019/11/IMG_9427_b.jpg">


The work consists of a series of „cards“ explaining different keywords: „RSSI“ (Radio Signal Strength Indicator), „Noise“, „Abrasion“, „Ageing“, „Silence“, „Signal“ and „Erosion“.

A NodeMCU (Microcontroller) is programmed to be in promiscious monitor mode(1) and look at all radio signals or „data packages“ floating through the air. The microcontroller will not make any attempts to connect to any WiFi hotspots, but rather just observe data packages from the outside and how strong their signal strength is. More data packages in closer proximity result in higher traffic around the microcontroller, which it has to experience. This tires out an internal „variable“ in the code and stresses the controller, thus starting to move the connected stepper motor in an irritating way.

The controller will only come down, once signals and noise around it vanish into silence – a perfectly calm equilibrium where nothing happens and moves. This state is hard to achieve and is best observed when no one is around.

––––––––––

(1) Based on the code „ESP8266 mini-sniff“ by Ray Burnette <a href="http://www.hackster.io/rayburne/projects">(link)</a>

<img src="https://www.julian-h.de/wp-content/uploads/2019/11/IMG_9335_b.jpg">
<img src="https://www.julian-h.de/wp-content/uploads/2019/11/IMG_9430_b.jpg">
<img src="https://www.julian-h.de/wp-content/uploads/2019/11/IMG_9324_bb.jpg">



# TMC2103 Motor driver
Before this project was published there was no "good" wiring diagram for Arduino and the TMC2103 stepper motor driver.
<img src="TMC2130-01.png">

# Exhibited
* 2021 – ‚Continuous Erosion‘ + ‚ASCII Animations – Splitflapesque‘, Ortstermin: Art festival in Moabit and Hansaviertel, Berlin. Germany with *Projektraum für analoge und digitale Experimente*: Thomas Brandstätter, Erik Freydank, Hojin Kang, Arnaud Korenke
* 2018 – ‚Continuous Erosion‘, (Un)natural encounters (Group Exhibition together with Irena Kukric, Bojana Petkovic, Mariana Schetini Basso, Luiz Zanotello), Galerie Herold, Bremen, Germany
* 2017 – ‚Viserosion‘, (Un)natural encounters (Group Exhibition together with Irena Kukric, Bojana Petkovic, Mariana Schetini Basso, Luiz Zanotello), Galeriji Doma kulture Studentski grad, Belgrade Serbia