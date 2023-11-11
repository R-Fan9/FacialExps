
# for i in range(35):

#     infile = f'/home/rfan/code/OpenGL/facialExps/data/faces/{i}.obj'
#     outfile = f'/home/rfan/code/OpenGL/facialExps/data/faces/test/{i}_t.obj'

#     with open(infile, 'r') as file:
#         with open(outfile, 'w') as output:
#             for line in file:
#                 if not line.startswith('f'):
#                     output.write(line)
#                 else:
#                     ls = line.split()
#                     if len(ls) < 5:
#                         output.write(line)
#                     else:
#                         ls1 = [s for s in ls[:4]]
#                         output.write(" ".join(ls1) + "\n")
#                         ls2 = [s for s in ls[:2]] + [s for s in ls[3:5]]
#                         output.write(" ".join(ls2) + "\n")


infile = f'/home/rfan/code/OpenGL/facialExps/data/faces/base.obj'
outfile = f'/home/rfan/code/OpenGL/facialExps/data/faces/test/base_t.obj'

with open(infile, 'r') as file:
    with open(outfile, 'w') as output:
        for line in file:
            if not line.startswith('f'):
                output.write(line)
            else:
                ls = line.split()
                if len(ls) < 5:
                    output.write(line)
                else:
                    ls1 = [s for s in ls[:4]]
                    output.write(" ".join(ls1) + "\n")
                    ls2 = [s for s in ls[:2]] + [s for s in ls[3:5]]
                    output.write(" ".join(ls2) + "\n")