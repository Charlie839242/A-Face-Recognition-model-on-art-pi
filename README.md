# A-Face-Recognition-model-on-art-pi

This model can identify the faces of a certain number of people depending on the labels that users give to the training set.
It mainly uses a two-layer CNN structure.

![image](https://github.com/Charlie839242/A-Face-Recognition-model-on-art-pi/blob/main/image/0.png)

# How to Use

  "py" folder contains codes that develops from taking photos to training a model.
        The whole process in "py" is mainly divided into 4 steps:
            1.Using 'taking_photos.py' to take photos of users and store them in "pictures" folder.
              You can decide the number of people that are going to be taken photo of and place them in different folders.
            2.Using 'pick_face.py' to capture the faces inside the taken photos and store them in "pic" folder.
            3.Using 'train_model.py' and dataset "pic" to train the model and store in "model" folder.
            4.Using 'keras2tflite_int_.py' to invert the obtained model into the form of tflite.

  "model" folder contains the resulted model.

  "BSP" folder contains the project files that are going to be employing on art-pi. It would betetr be opened by "RT-Thread Studio" as a project.
  

