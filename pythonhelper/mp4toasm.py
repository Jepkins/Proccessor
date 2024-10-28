import cv2 as cv
import argparse
import math

SCREEN_WIDTH = 100
SCREEN_HEIGHT = 100

VIDEO_WIDTH = 200
VIDEO_HEIGHT = 150

def get_int_color(bgr, do_bnw):
    blue = min(bgr[0], 255)
    green = min(bgr[1], 255)
    red = min(bgr[2], 255)
    if do_bnw:
        if blue | green | red > 100:
            return 0xff000000
        else:
            return 0xffffffff

    return int((255 << 24) | (blue << 16) | (green << 8) | red)

def process_video(input_file, output_file, do_bnw):
    video_capture = cv.VideoCapture(input_file)

    fps = int(video_capture.get(cv.CAP_PROP_FPS))
    delay = 1000 // fps
    frame_count = int(video_capture.get(cv.CAP_PROP_FRAME_COUNT))
    current_colors = [None] * (SCREEN_WIDTH * SCREEN_HEIGHT)

    output = []

    for frame_index in range(frame_count): # MIND: frame_count
        video_capture.set(cv.CAP_PROP_POS_FRAMES, frame_index)
        success, frame = video_capture.read()

        if not success:
            continue

        frame = cv.resize(frame, (SCREEN_WIDTH, SCREEN_HEIGHT), interpolation=cv.INTER_LANCZOS4)
        for y in range(SCREEN_WIDTH):
            for x in range(SCREEN_HEIGHT):
                bgr_color = frame[x, y]
                color = get_int_color(bgr_color, do_bnw)

                index = y * SCREEN_HEIGHT + x

                if current_colors[index] != color:
                    output.append(f"mov [{index}] {color}")
                    current_colors[index] = color

        output.append("draw")
        output.append(f"sleep {delay}")
    output.append("hlt")

    with open(output_file, "w") as file:
        file.write("\n".join(output))

    video_capture.release()

ap = argparse.ArgumentParser()
ap.add_argument("-b", "--black_n_white",  required=False, help="path to input video")
ap.add_argument("-v", "--video",   required=True, help="path to input video")
ap.add_argument("-o", "--outfile", required=True, help="path to output asm")
args = vars(ap.parse_args())
input_filename = args["video"]
output_filename = args["outfile"]
do_bnw = args["black_n_white"]
process_video(input_filename, output_filename, do_bnw)
