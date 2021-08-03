# A-Face-Recognition-model-on-art-pi

It mainly uses a two-layer CNN structure to identify faces of different people.

"py" file contains codes that develops from taking photos to training a model.
      The whole process in "py" is mainly divided into 4 steps:
          1.Using 'taking_photos.py' to take photos of users and store them in "pictures" file.
          2.Using 'pick_face.py' to capture the faces inside the taken photos and store them in "pic" file.
          3.Using 'train_model.py' and dataset "pic" to train the model in "model" file.
          4.Using 'keras2tflite_int_.py' to invert the obtained model into the form of tflite.
          
"model" file contains the resulted model.

"BSP" file contains the project files that are going to be employing on art-pi. It would betetr be opened by "RT-Thread Studio" as a project.
  

