The whole process is mainly divided into 4 steps:
	1.Using 'taking_photos.py' to take photos of users and store them in "pictures" file.
	2.Using 'pick_face.py' to capture the faces inside the taken photos and store them in "pic" file.
	3.Using 'train_model.py' and dataset "pic" to train the model in "model" file.
	4.Using 'keras2tflite_int_.py' to invert the obtained model into the form of tflite.