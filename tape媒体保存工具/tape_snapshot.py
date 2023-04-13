import os
import re
import time
import shutil
import requests

from tqdm import tqdm

source_path = 'D:/Torrent/15955897608' # 设置为导出的手机号码目录路径
target_path = None

def save_batch(text, work_dir, urls, static_path, sleep_time=0.2):
    if len(urls) <= 0:
        return text
    os.chdir(work_dir)
    for url in tqdm(urls, ncols=60):
        filename = url.split('/')[-1]
        response = requests.get(url)
        with open(filename, 'wb') as f:
            f.write(response.content)
        time.sleep(sleep_time)
        text = text.replace(url, os.path.join(static_path, filename))
    return text

def save(userdir):
    tasks = []  # [(source_html_path, target_html_path staic_resource_relative_path)]
    for base, dirs, files in os.walk(userdir):
        target_base = base.replace(source_path, target_path)
        os.makedirs(target_base, exist_ok=True)
        for filename in files:
            if filename.endswith('.html'):
                tasks.append((os.path.join(base, filename), os.path.join(target_base, filename), 
                             '.' if base == userdir else '..'))
    
    target_user_dir = userdir.replace(source_path, target_path)
    pic_dir = os.path.join(target_user_dir, 'picture')
    audio_dir = os.path.join(target_user_dir, 'audio')
    video_dir = os.path.join(target_user_dir, 'video')
    os.makedirs(pic_dir, exist_ok=True)
    os.makedirs(audio_dir, exist_ok=True)
    os.makedirs(video_dir, exist_ok=True)
    
    cnt = 0
    for shtml, thtml, static_path in tasks:
        cnt += 1
        print(f'正在处理{shtml}，第{cnt}个文件，共{len(tasks)}个文件')
        with open(shtml, 'r', encoding='utf8') as source, open(thtml, 'w', encoding='utf8') as target:
            text = source.read()
            
            img_urls = re.findall(r'<img.*?src="(.*?)".*?>', text, re.S)
            img_urls = [url.split('?')[0] for url in img_urls]
            text = save_batch(text, pic_dir, img_urls, os.path.join(static_path, 'picture'))
            
            audio_urls = re.findall(r'<audio.*?<source src="(.*?)".*?>', text, re.S)
            text = save_batch(text, audio_dir, audio_urls, os.path.join(static_path, 'audio'))
            
            video_urls = re.findall(r'<video.*?<source src="(.*?)".*?>', text, re.S)
            text = save_batch(text, video_dir, video_urls, os.path.join(static_path, 'video'))
            
            target.write(text)
            print(f'{shtml}处理完毕，转换后的新文件为{thtml}')


if __name__ == '__main__':
    try:
        source_path = os.path.join(os.path.abspath(os.curdir), source_path)
        if os.path.isdir(source_path):
            target_path = source_path
            while target_path.endswith('/') or target_path.endswith('\\'):
                target_path = target_path[:-1]
            target_path += '_local'
            # shutil.copytree(source_path, target_path)
            
            print(f'\n新建目录{target_path}\n\n所有图像/音频/视频数据将保存在各用户目录下的picture/audio/video文件夹下')
            time.sleep(2)
            print('\n下载需要时间，请耐心等待\n')
            time.sleep(2)
            
            for userdir in os.listdir(source_path):
                print(f'处理用户{userdir}')
                save(os.path.join(source_path, userdir))
                print(f'用户{userdir}处理完毕')
            
            print(f'\033[0;36;40m保存完毕，祝你天天开心！\033[0m')
        
        else:
            print(f'未找到tape目录！请按以下步骤执行:\n')
            print('\n1. 解压tape导出的zip文件，将解压出的手机号码文件夹重命名为tape\n')
            print('\n2. 将本exe文件与重命名为tape的文件夹放置于同一目录下\n\t例如D:\\tape 和 D:\\tape_snapshot.exe\n\n')
    except Exception as e:
        print('出现错误：', e)
        print('\033[0;31;40m可联系程序作者\033[0m')
        
            
    input('\n按任意键关闭本窗口...')