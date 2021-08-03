print("正在初始化摄像头...")
import cv2
import os
import datetime

cap = cv2.VideoCapture(0)
print("初始化成功！")

# name='play_phone'
# name='haqian'
# name='spleeing'
# name='zhengchang'
# name="zhedang"
name = "LYC"
savedpath = r'./pictures/' + name
isExists = os.path.exists(savedpath)
if not isExists:
    os.makedirs(savedpath)
    print('path of %s is build' % (savedpath))
else:
    print('path of %s already exist and rebuild' % (savedpath))
print("按N键拍摄图片")
i = 0
while (True):
    ret, frame = cap.read()
    gray = cv2.cvtColor(frame, 1)
    cv2.imshow('test', frame)                   #test是窗口名称
    now = datetime.datetime.now()
    now = now.strftime('%m-%d-%H-%M-%S')
    savedname = '/' + name + '_' + str(i) + '_{0}' '.jpg'.format(now)
    if cv2.waitKey(1) & 0xFF == ord('n'):  # 按N拍摄
        i += 1
        cv2.imwrite(savedpath + savedname, frame)
        cv2.namedWindow("Image")
        cv2.imshow("Image", frame)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

cap.release()
cv2.destroyAllWindows()