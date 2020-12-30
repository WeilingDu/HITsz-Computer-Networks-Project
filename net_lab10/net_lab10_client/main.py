import client


if __name__ == '__main__':
    print('We provide the following two functions: ')
    print('1. Upload file ')
    print('2. Download file \n')
    print('Please input the function number you want: (eg. 1) ')
    option = input()
    if option == '1':
        print('Please input the file name you want to upload: (eg. Goodbye.txt) ')
    else:
        print('Please input the file name you want to download: (eg. Goodbye.txt) ')
    filename = input()
    if option == '1':
        print('Upload... ')
        client.upload_file(filename)
    else:
        print('Download... ')
        client.download_file(filename)
