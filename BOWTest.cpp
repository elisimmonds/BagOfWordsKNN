//predicted what images are from a test set based on training set
//no arguments, change directories for train and test in code

# include <opencv2/opencv_modules.hpp>
# include <stdio.h>

# include <opencv2/core/core.hpp>
# include <opencv2/features2d/features2d.hpp>
# include <opencv2/imgproc/imgproc.hpp>
# include <opencv2/highgui/highgui.hpp>
# include <opencv2/nonfree/features2d.hpp>

# include <opencv2/legacy/legacy.hpp>
# include <stdlib.h>

# include <opencv2/opencv.hpp>
# include <fstream>
# include <iostream>
# include <string>
# include <string.h>

# include <dirent.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/types.h>

using namespace cv;
using namespace std;

int main (int argc, char * const argv[]) {
    string dir = "/Users/elisimmonds/Code/IndependentStudy/LarvaeClassification/Images/OriginalRes/train/", filepath; //Directory for training images
    DIR *dp;
    struct dirent *dirp;
    struct stat filestat;

    dp = opendir( dir.c_str() );

    // Create the object for the vocabulary

    BOWKMeansTrainer bowtrainer(10); //num clusters
    
    // detecting keypoints
    SurfFeatureDetector detector(400);
    // OrbFeatureDetector detector(100); //Using multiple feature detectors
    // MserFeatureDetector detector(400);
    // SiftFeatureDetector detector2(100);

    
    // computing descriptors
    Ptr<DescriptorExtractor > extractor(new SiftDescriptorExtractor());//  extractor;
    Mat training_descriptors(1, extractor->descriptorSize(), extractor->descriptorType());

    
    cout << "------- build vocabulary ---------\n";

    cout << "extract descriptors.."<<endl;
    int count = 0;
    while ((dirp = readdir( dp ))) //Reading all files in train directory
    {
        vector<KeyPoint> keypoints; 

        Mat descriptors;

        filepath = dir + "/" + dirp->d_name;
        
        // If the file is a directory (or is in some way invalid) we'll skip it 
        if (stat( filepath.c_str(), &filestat )) continue;
        if (S_ISDIR( filestat.st_mode ))         continue;
        if (dirp->d_name[0] == '.')                  continue; //hidden files! 

        Mat img = imread(filepath, 1);

        // vector<KeyPoint> keypoints1; //Using multiple feature detectors
        // vector<KeyPoint> keypoints2; 

        detector.detect(img, keypoints);
        // detector1.detect(img, keypoints1);
        // detector2.detect(img, keypoints2);

        // keypoints.insert(keypoints.end(), keypoints1.begin(), keypoints1.end()); //Merging multiple feature detectors
        // keypoints.insert(keypoints.end(), keypoints2.begin(), keypoints2.end());

        extractor->compute(img, keypoints, descriptors);
        training_descriptors.push_back(descriptors);
    }
    cout << endl;
    closedir( dp );

    cout << "Total descriptors: " << training_descriptors.rows << endl;
    

    bowtrainer.add(training_descriptors);
    cout << "cluster BOW features" << endl;



    Mat vocabulary = bowtrainer.cluster(); //Create the vocabulary with KMeans. 

    Mat trainData(0, vocabulary.size, extractor->descriptorType());
    Mat labels(0, 0, extractor->descriptorType());

    //Now, you have a vocabulary, compute the occurence of delegate in object
    Ptr<DescriptorMatcher > matcher(new BruteForceMatcher<L2<float> >());
    BOWImgDescriptorExtractor dextract(extractor, matcher);

    //Set the vocabulary
    dextract.setVocabulary(vocabulary);
    count = 0;

    dp = opendir( dir.c_str() );    
    while ((dirp = readdir( dp ))) //All files in training directory
    {
        vector<KeyPoint> keypoints; 
        Mat histogram;

        filepath = dir + "/" + dirp->d_name;
        if (stat( filepath.c_str(), &filestat )) continue;
        if (S_ISDIR( filestat.st_mode ))         continue;
        if (dirp->d_name[0] == '.')                  continue; //hidden file! 
        
        Mat img = imread(filepath);

        // vector<KeyPoint> keypoints1; //Using multiple feature detectors
        // vector<KeyPoint> keypoints2; 

        detector.detect(img, keypoints);
        // detector1.detect(img, keypoints1);
        // detector2.detect(img, keypoints2);

        // keypoints.insert(keypoints.end(), keypoints1.begin(), keypoints1.end()); //Merging multiple feature detectors
        // keypoints.insert(keypoints.end(), keypoints2.begin(), keypoints2.end());


        dextract.compute(img, keypoints, histogram);

        count++;
        cout << "count: " << count << endl;
        trainData.push_back(histogram); //trainData is a vector-like for SVM, histogram data for each images
        cout << "traindata rows:" << trainData.rows << endl;

        char fileLabel = dirp->d_name[0];

        cout << "filename: " << dirp->d_name << endl;
        if(fileLabel == 'A') { //Image names were labeled with A or N on whether it was Abalone or Nucella (EX: A1298.png)
            labels.push_back((float)0);
        } else {
            labels.push_back((float)1);
        }
    }

    closedir(dp);

    cout << "traindata rows:" << trainData.rows << endl;
    cout << "traindata cols:" << trainData.cols << endl;


    cout << "label rows:" << labels.rows << endl;
    cout << "label cols:" << labels.cols << endl;



    cout << "------- train SVMs ---------\n";

    // Default svm parameters
    // CvSVM svm;
    // CvSVMParams params;
    // svm.train(trainData, labels, Mat(), Mat(), params);
    // params = svm.get_params(); 

    // Create custom paramgrids
    // CvParamGrid CvParamGrid_C(pow(2.0,-5), pow(2.0,15), pow(2.0,1));
    // CvParamGrid CvParamGrid_gamma(pow(2.0,-5), pow(2.0,15), pow(2.0,1));
    // CvParamGrid CvParamGrid_P(pow(2.0,-5), pow(2.0,15), pow(2.0,2));
    // CvParamGrid CvParamGrid_nu(0.1, 1, .1);
    // CvParamGrid CvParamGrid_coef(pow(2.0,-5), pow(2.0,15), pow(2.0,2));
    // CvParamGrid CvParamGrid_degree(1, 10, 1);

    
    int K = 5;
    CvKNearest knn(trainData, labels, Mat(), false, K);
    
    knn.train(trainData, labels, Mat(), false, K, false);   // 5 is how many nearest neighbors we want.
    // train(const Mat& trainData, const Mat& responses, const Mat& sampleIdx=Mat(), bool isRegression=false, int maxK=32, bool updateBase=false
    
    CvMat* nearests = cvCreateMat( 1, K, CV_32FC1);
    
    

//    params = svm.get_params();

//    cout<<"gamma:"<<params.gamma<<endl;
//    cout<<"C:"<<params.C<<endl;
//    cout<<"P:"<<params.p<<endl;
//    cout<<"nu:"<<params.nu<<endl;
//    cout<<"coef:"<<params.coef0<<endl;
//    cout<<"degree:"<<params.degree<<endl;

    // svm.save("svmNewPics.xml"); //Save and load svm data
    // svm.load("svmNewPics.xml");

    cout << "------- test -------------- \n";


    dir = "/Users/elisimmonds/Code/IndependentStudy/LarvaeClassification/Images/OriginalRes/test/"; //Sets directory to test images

    dp = opendir( dir.c_str() ); 

    float totalN = 0;
    float accN = 0;
    float totalA = 0;
    float accA = 0;

    while ((dirp = readdir( dp )))
    {
        vector<KeyPoint> keypoints; 
        Mat histogram;

        filepath = dir + "/" + dirp->d_name;
        if (stat( filepath.c_str(), &filestat )) continue;
        if (S_ISDIR( filestat.st_mode ))         continue;
        if (dirp->d_name[0] == '.')                  continue; //hidden file! 

        Mat img = imread(filepath);

        // vector<KeyPoint> keypoints1; 
        // vector<KeyPoint> keypoints2; //Multiple detectors

        detector.detect(img, keypoints);
        // detector1.detect(img, keypoints1);
        // detector2.detect(img, keypoints2);

        // keypoints.insert(keypoints.end(), keypoints1.begin(), keypoints1.end());
        // keypoints.insert(keypoints.end(), keypoints2.begin(), keypoints2.end());

        dextract.compute(img, keypoints, histogram);

        CvMat cvHistogram = histogram;
        float predicted = knn.find_nearest(&cvHistogram, K, 0, 0, nearests, 0);

        // Print predicted results for each image
        if (predicted == 0) {
            char fileLabel = dirp->d_name[0];
            if(fileLabel == 'A') {
                totalA += 1;
                accA += 1;
            } else {
                totalN += 1;
            }
            cout << dirp->d_name << " predicted to be: Abalone" << endl;
        } else {
            char fileLabel = dirp->d_name[0];
            if(fileLabel == 'N') {
                totalN += 1 ;
                accN += 1;
            } else {
                totalA += 1;
            }
            cout << dirp->d_name << " predicted to be: Nucella" << endl;
        }
    }

    float nPercent = accN/totalN;
    float aPercent = accA/totalA;

    cout << "Correct percent of Abalone Images: " << aPercent << endl;
    cout << "Correct percent of Nucella Images: " << nPercent << endl;


    return 0;
}
