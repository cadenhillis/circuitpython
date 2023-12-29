if __name__ == "__main__":
    with open("extmod/ulab/code/numpy/io/io.c", "r+") as f:
        file = f.read()
        file_split = file.split('\n')
        print('file_split', file_split)
#        print('file', file)
        
        for i, line in enumerate(file_split):
            index = line.find('read(')
#            print('index', index)
#            print('line', line)
            if index != -1:
                last_comma = line.rfind('error') + 5
                file_split[i] = line[:last_comma] + ',0' + line[last_comma:]
                print('fixed line', line)
                print('now:', file_split[i])
        
    with open("extmod/ulab/code/numpy/io/io.c", "w") as f:
        f.write('\n'.join(file_split))
#        print('file_split', file_split)
#        print('\n'.join(file_split))
        #f.write(file)
