#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <string>

enum ImacqMethod
{
    IMACQ_IMGS, //!< Images
    IMACQ_CAM, //!< Camera
    IMACQ_VID, //!< Video
    IMACQ_LIVESIM, //!< Livesim
    IMACQ_STREAM //!< Stream
};

//#include "../libconfig/libconfig.h++"

//#include "Settings.h"
#include "Main.h"

//namespace tld
//{

/**
 * In this class all settings are stored.
 */
class Settings
{
public:
    /**
     * Standard-Constructor
     */
    Settings() {
    	    m_useProportionalShift = true;
    	    m_varianceFilterEnabled = true;
    	    m_ensembleClassifierEnabled = true;
    	    m_nnClassifierEnabled = true;
    	    m_loadModel = false;
    	    m_trackerEnabled = true;
    	    m_selectManually = false;
    	    m_learningEnabled = true;
    	    m_showOutput = true;
    	    m_showNotConfident = true;
    	    m_showColorImage = false;
    	    m_showDetections = false;
    	    m_showForeground = false;
    	    m_saveOutput = false;
    	    m_alternating = false;
    	    m_exportModelAfterRun = false;
    	    m_trajectory = 0;
    	    m_method = IMACQ_CAM;
    	    m_startFrame = 1;
    	    m_lastFrame = 0;
    	    m_minScale = -10;
    	    m_maxScale = 10;
    	    m_numFeatures = 13;
    	    m_numTrees = 10;
    	    m_thetaP = 0.65;
    	    m_thetaN = 0.5;
    	    m_minSize = 25;
    	    m_camNo = 0;
    	    m_fps = 24;
    	    m_seed = 0;
    	    m_threshold = 0.7;
    	    m_proportionalShift = 0.1;
    	    m_modelExportFile = "model";
    };
    ~Settings() {};
    bool m_trackerEnabled;
    bool m_varianceFilterEnabled;
    bool m_ensembleClassifierEnabled;
    bool m_nnClassifierEnabled;
    bool m_useProportionalShift; //!< sets scanwindows off by a percentage value of the window dimensions (specified in proportionalShift) rather than 1px.
    bool m_loadModel; //!< if true, model specified by "modelPath" is loaded at startup
    bool m_selectManually; //!< if true, user can select initial bounding box (which then overrides the setting "initialBoundingBox")
    bool m_learningEnabled; //!< enables learning while processing
    bool m_showOutput; //!< creates a window displaying results
    bool m_showNotConfident; //!< show bounding box also if confidence is low
    bool m_showColorImage; //!< shows color images instead of greyscale
    bool m_showDetections; //!< shows detections
    bool m_showForeground; //!< shows foreground
    bool m_saveOutput; //!< specifies whether to save visual output
    bool m_alternating; //!< if set to true, detector is disabled while tracker is running.
    bool m_exportModelAfterRun; //!< if set to true, model is exported after run.
    int m_trajectory; //!< specifies the number of the last frames which are considered by the trajectory; 0 disables the trajectory
    int m_method; //!< method of capturing: IMACQ_CAM, IMACQ_IMGS or IMACQ_VID
    int m_startFrame; //!< first frame of capturing
    int m_lastFrame; //!< last frame of caputing; 0 means take all frames
    int m_minScale; //!< number of scales smaller than initial object size
    int m_maxScale; //!< number of scales larger than initial object size
    int m_numFeatures; //!< number of features
    int m_numTrees; //!< number of trees
    float m_thetaP;
    float m_thetaN;
    int m_seed;
    int m_minSize; //!< minimum size of scanWindows
    int m_camNo; //!< Which camera to use
    float m_fps; //!< Frames per second
    float m_threshold; //!< threshold for determining positive results
    float m_proportionalShift; //!< proportional shift
    std::string  m_imagePath; //!< path to the images or the video if m_method is IMACQ_VID or IMACQ_IMGS
    std::string m_modelPath; //!< if modelPath is not set then either an initialBoundingBox must be specified or selectManually must be true.
    std::string m_modelExportFile; //!< Path where model is saved on export.
    std::string m_outputDir; //!< required if saveOutput = true, no default
    std::string m_printResults; //!< path to the file were the results should be printed; NULL -> results will not be printed
    std::string m_printTiming; //!< path to the file were the timings should be printed; NULL -> results will not be printed
    std::vector<int> m_initialBoundingBox; //!< Initial Bounding Box can be specified here
};

/**
 * Config is used to configure the program by cli and/or by a config file
 */
class Config
{
public:
    /**
     * Constructor
     */
    Config();

    //Config(Settings &settings);

    /**
     * Destructor
     */
    ~Config();

    /**
     * Initializes the Config.
     * @param argc number of command line arguments
     * @param argv command line arguments
     * @return PROGRAM_EXIT when an error occurred, SUCCESS if not
     */
    int init(int argc, char **argv);

    /**
     * Configures the CamNode, ImAcq and TldClassifier.
     * @param node
     * @param imAcq
     * @param classifier
     * @return SUCCESS
     */
    int configure(Main *main);
private:
    //libconfig::Config m_cfg; //!< libconfig++
    //std::string m_configPath; //!< path to the config file
    Settings m_settings; //!< adjusted settings
    ///@{
    /**
     * Flags if parameters were set by cli-arguments.
     * When the flags are set, the parameters couldn't be
     * overwritten by the options in the config file.
     * The cli-arguments have a higher priority than the options
     * in the config file.
     */
    bool m_selectManuallySet;
    bool m_methodSet;
    bool m_startFrameSet;
    bool m_lastFrameSet;
    bool m_trajectorySet;
    bool m_showDetectionsSet;
    bool m_showForegroundSet;
    bool m_thetaSet;
    bool m_printResultsSet;
    bool m_camNoSet;
    bool m_imagePathSet;
    bool m_modelPathSet;
    bool m_initialBBSet;
    bool m_showOutputSet;
    bool m_exportModelAfterRunSet;
    ///@}
};


/*
 POSIX getopt for Windows

 AT&T Public License

 Code given out at the 1985 UNIFORUM conference in Dallas.
 */
#ifdef __GNUC__
#   include <getopt.h>
#endif

#ifndef __GNUC__
extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;
extern int getopt(int argc, char **argv, char *opts);
#endif  /* __GNUC__ */

//} /* End Namespace */


#endif /* CONFIG_H */
