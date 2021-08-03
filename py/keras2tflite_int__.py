import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
import cv2
import time
import random
from tqdm import tqdm
import numpy as np
import tensorflow as tf
from pathlib import Path

def create_test_data(CATEGORIES, DATADIR, img_shape):
    test_x = []  # 测试集的数据和标签
    for category in CATEGORIES:
        path = os.path.join(DATADIR, category)
        for img in tqdm(os.listdir(path)):  # iterate over each image
            try:
                img_array = cv2.imread(os.path.join(path, img))  # convert to array
                img_array = cv2.cvtColor(img_array, cv2.COLOR_BGR2RGB)
                new_array = cv2.resize(img_array, img_shape)  # resize to normalize data size
                new_array = new_array.astype(np.float32) / 255.
                new_array = np.expand_dims(new_array, axis=0)
                test_x.append(new_array)
            except Exception as e:  # in the interest in keeping the output clean...
                pass

    return test_x


def keras2tflite(keras_file, tflite_file, test_images):
    def representative_data_gen():
        for input_value in test_images:
            yield [input_value]
            
    # 恢复 keras 模型，并预测
    model = tf.keras.models.load_model(keras_file)

    # 动态量化 dynamic range quantization
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_data_gen
    # Ensure that if any ops can't be quantized, the converter throws an error
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    # Set the input and output tensors to uint8 (APIs added in r2.3)
    converter.inference_input_type = tf.uint8
    converter.inference_output_type = tf.uint8
    
    tflite_model = converter.convert()
    
    tflite_file.write_bytes(tflite_model)
    print("convert model to tflite done...")



def tflite_infer(tflite_file, image_path, CATEGORIES):
    # tflite 模型推理
    interpreter = tf.lite.Interpreter(model_path=str(tflite_file))
    interpreter.allocate_tensors()

    # Get input and output tensors.
    input_details = interpreter.get_input_details()[0]
    output_details = interpreter.get_output_details()[0]
    height = input_details['shape'][1]
    width = input_details['shape'][2]

    # 单个测试样本数据
    image = cv2.imread(image_path)
    image = cv2.resize(image, (width, height))
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    image = np.expand_dims(image, axis=0)
    image.flatten()
    interpreter.set_tensor(input_details['index'], image)

    start_time = time.time()
    interpreter.invoke()
    stop_time = time.time()

    # interpreter = tf.lite.Interpreter(model_content=tflite_model)
    input_type = interpreter.get_input_details()[0]['dtype']
    print('input: ', input_type)
    output_type = interpreter.get_output_details()[0]['dtype']
    print('output: ', output_type)

    output_data = interpreter.get_tensor(output_details['index'])
    print(output_data)
    print(f"prediction: {CATEGORIES[np.argmax(output_data)]}")
    print('time: {:.3f}ms'.format((stop_time - start_time) * 1000))
    print("model size: {:.2f} MB".format(os.path.getsize(tflite_file)/1024/1024))
    

def main():
    DATADIR = 'test'
    CATEGORIES = ['LY', 'LYC', 'LZY']
    img_shape = (128, 128)
    keras_file = 'model/model.h5'
    tflite_file = Path("model/model.tflite")
    test_image_path = "pic/LYC/16266854601.jpg"

    test_x = create_test_data(CATEGORIES, DATADIR, img_shape)
    keras2tflite(keras_file, tflite_file, test_x)
    tflite_infer(tflite_file, test_image_path, CATEGORIES)


if __name__ == "__main__":
    main()