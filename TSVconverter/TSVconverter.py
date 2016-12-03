import os
from PIL import Image
 
'''
Most of this code is from https://gist.github.com/BigglesZX/4016539

Determined that analysing the GIF to see if there are transparent update frames was implemented
incorrectly- full frame updates not detected, so for now we'll just start with the last frame,
if there's no transparency, the whole thing will be overwritten anyway. Seems to work fine.
'''
 
def processImage(path):
    '''
    Iterate the GIF, extracting each frame.
    '''
    
    im = Image.open(path)
 
    i = 0
    p = im.getpalette()
    last_frame = im.convert('RGBA')

    out = open(os.path.basename(path).split('.')[0]+'.tsv','wb')
    
    try:
        while True:
            #print "saving %s (%s) frame %d, %s %s" % (path, mode, i, im.size, im.tile)
            
            '''
            If the GIF uses local colour tables, each frame will have its own palette.
            If not, we need to apply the global palette to the new frame.
            '''
            if not im.getpalette():
                im.putpalette(p)
            
            new_frame = Image.new('RGBA', im.size)
            
            '''
            We need to construct the new frame by pasting it on top of the preceding frames.
            '''
            new_frame.paste(last_frame)

            '''
            Sometimes trasnparency is not captured correctly with RGBA conversion. This seems to catch most.
            '''

            if 'transparency' in im.info:
                transparency = im.info['transparency']
                #print 'frame %d transparency found = %d' % (i,transparency)
                colorTable = [255]*256
                colorTable[transparency] = 0
                mask=im.point(colorTable,'1')
                new_frame.paste(im, (0,0), mask)
            else:
                new_frame.paste(im, (0,0), im.convert('RGBA'))
            
            #new_frame.save('%s-%d.png' % (''.join(os.path.basename(path).split('.')[:-1]), i), 'PNG')
 
            i += 1
            last_frame = new_frame

            '''
            TinyScreen raw pixel data output section
            '''
            
            if im.size[0]!=96 or im.size!=64:
                new_frame=new_frame.resize((96,64), Image.ANTIALIAS)
            
            data = list(new_frame.convert('RGB').getdata())
            raw=[]
            for j in range(96*64):
                raw.append(((data[j][2]>>0)&0xE0)|((data[j][1]>>3)&0x1C)|((data[j][0]>>6)&0x03))
            out.write(bytearray(raw))
            
            '''
            do this last
            '''
            im.seek(im.tell() + 1)
    except EOFError:
        pass
    out.close()
 
def main():
    for file in os.listdir('.'):
        if file.endswith('.gif'):
            processImage(file)
    
 
if __name__ == "__main__":
    main()
