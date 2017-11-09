# README #

This README would normally document whatever steps are necessary to get your application up and running.

### Module intriduction ###

* This project include three parts: 
  1. TrackingManager
  2. Tracking algorithm module "TrackAlgTLD".
  3. An analyzer "AnalyzerTK" for showing the result.
  
* TrackingManager:
    * Responsible for maintaining all resources that will be dispatched to all cameras(e.g. IP camera) in the system, including the object feature transferring and the data format definition etc.

* The Tracking Algorithm "TrackAlgTLD" implemented with an open source framework TLD(tracking-learning-detection) at present, and was built as a library for easy to replace.

* Other tracking algorithm implemented by yourself can replace the tracking module. 
    * Just build your tracking library with the name prefix "TrackAlg", and don't forget to inherit the TrackAlg class. For instance, in this case, "TrackAlgTLD".

* Finally, AnalyzerTK will invoke the tracking APIs for the intra camera tracking and showing the results by use of openCV.

* If you want to build a cross camera system, please execute AnalyzerTK once for each camera and input different camera IDs and URLs as parameters described in the section of parameter settings. Each AnalyzerTK will transfer the feature of targets to all other tracking process for re-identification.

### How do I get set up? ###

* To run this system, make sure that you have already installed openCV (building with gstreamer and without ffmpeg, since ffmpeg has a socket buffer only 64KB and may cause the packet loss) version with 2.4.9 or later.
* First, you should run TrackingManager to create the resource for the system.
* Ex: TrackingManager -t 2 -m 4, which "-t" means how many cameras will be used and "-m" the max number of targets each camera can support.
* After running TrackingManager, you can execute AnalyzerTK for tracking.

### Parameter settings for AnalyzerTK###

* "-c" : give the camera ID in integer.
* "-k" : set up an interval in ms to keep tracking an object which disappeared from that moment.
* "-l" : the detection model file path trained by adaboost(openCV).
* "-r" : describe which tracking algorithm will be loaded (in this system will be "TLD").
* "-v" : set up the IP camera URL.

### For imrpovement###

* Now, this algorithm seems not good for object re-identification in multi-object tracking since its feature is not robust(only color histogram). I will try to add some new features or use other algorithm such as ELM for making it better.

* The entry/exit zone mapping function(algorithm) for cross camera tracking will be implemented in this module.

* This framework is just implemented under the local host scenario at present and will be built with a decentralized architecture.
