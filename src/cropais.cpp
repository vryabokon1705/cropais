//============================================================================
// Name        : cropais.cpp
// Author      : Vsevolod Ryabokon
// Version     :
// Copyright   : 
// Description : Crop annotated images
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

using namespace std;


vector<int> getObjPosition(const vector<string> &imgAnnotationsVec, int objNr){
	vector<int> result;
	int startAnnotPosition=objNr*4+2;

	for(int i=0 ; i<4; i++){
		result.push_back(atoi(imgAnnotationsVec[startAnnotPosition+i].c_str()));
	}
	return result;
}

void cropImage(string line, string annotation_file_name, int w, int h){

	boost::filesystem::path p(annotation_file_name);
	boost::filesystem::path dir = p.parent_path();

	boost::filesystem::path croppedFilesDir(dir.generic_string()+"/cropped");

	if(!boost::filesystem::exists(croppedFilesDir)){
		boost::filesystem::create_directory(croppedFilesDir);
	}
	else if(!boost::filesystem::is_directory(croppedFilesDir)){
		cout<<"File exists and not directory: "<<croppedFilesDir<<", exiting!"<<endl;
		return;
	}


	vector<string> imgAnnotationsVec;
	boost::split(imgAnnotationsVec, line, boost::is_space(), boost::token_compress_on);

	if(imgAnnotationsVec.size()>=6) {
		//annotationLine

		string imagFileName=imgAnnotationsVec[0];



		boost::filesystem::path imagFile(imagFileName);



		cout<<"Image file name: "<<imagFileName<<endl;

		int numberOfObjects=atoi(imgAnnotationsVec[1].c_str());

		cv::Mat img = cv::imread(imagFileName);


		if (img.empty())
		{
			std::cout << "!!! imread() failed to open target image" << std::endl;
			return;
		}




			for(int objNr=0;objNr<numberOfObjects; objNr++ ){

				vector<int> objPosition=getObjPosition(imgAnnotationsVec, objNr);

				cout<<"object:  "<<objNr<<endl;

				if(objPosition.size()==4){
					cout<<"object position:"<<endl;
					cout<<"      ";
					for(int i=0; i<objPosition.size(); i++){
						cout<<" ["<<i<<"]:"<<objPosition[i];
					}
					cout<<endl;

					cv::Rect roi;
					roi.x=objPosition[0];
					roi.y=objPosition[1];
					roi.width=objPosition[2];
					roi.height=objPosition[3];

					float scale=1;
					if(w>0&&h>0){
						float scaleW=(float)w/roi.width;
						float scaleH=(float)h/roi.height;
						scale=(scaleW>scaleH?scaleW:scaleH);
					}


					if(roi.x>=0&&roi.y>=0&&roi.width>0&&roi.height>0&&(roi.x+roi.width)<=img.cols&&(roi.y+roi.height)<=img.rows ){
						cv::Mat crop1=img(roi);

						cv::Mat crop2;
						cv::Size size(roi.width*scale, roi.height*scale);

						cv::resize(crop1, crop2, size, (double)scale, (double)scale);

						int cropX=0;
						int cropY=0;
						//Crop to size
						if(crop2.cols>w){
							//Crop to width

							cropX=(crop2.cols-w)/2;

						}
						if(crop2.rows>h){
							//Crop to height
							cropY=(crop2.rows-h)/2;
						}

						roi.x=cropX;
						roi.y=cropY;
						roi.width=w;
						roi.height=h;

						cv::Mat crop=crop2(roi);

						stringstream croppedImgFileSS;

						croppedImgFileSS<<croppedFilesDir.string()<<"/"<<objNr<<'_'<<imagFile.filename().string();

						string croppedImgFile=croppedImgFileSS.str();

						imwrite(croppedImgFileSS.str(),crop);
					}
					else{
						cout<<"Cannot crop image. Size miss! "<<endl;
					}



				}
				else{
					cout<<"Wrong object position!: < "<<endl;
					cout<<"      ";
					for(int i=0; i<objPosition.size(); i++){
						cout<<"["<<i<<"] "<<objPosition[i];
					}
					cout<<endl;
				}

			}

	}
	else{
		cout<<"No annotation line!: < "<<line<<" >"<<endl;
	}




}



int main(int argc, char* argv[]) {

	//argv[1] - annotations file
	//argv[2] - width (new)
	//argv[3] - height (new)
	ifstream annotation_file;
	string annotation_file_name;
	string line;
	int w=0, h=0;

	if(argc>1 )
		annotation_file_name=argv[1];
	else{
		cout<<"Please provide annotation file name"<<endl;
		cin>>annotation_file_name;
	}

	if(argc>2 ){
		w=atoi(argv[2]);
	}

	if(argc>3 ){
		h=atoi(argv[3]);
	}

	annotation_file.open(annotation_file_name, ios::in);

	if (annotation_file.is_open())
	{
		while ( getline (annotation_file,line) )
		{
			cropImage(line, annotation_file_name, w, h);

		}
		annotation_file.close();
	}

	else cout << "Unable to open file";

	cout << "" << endl; // prints 
	return 0;
}


