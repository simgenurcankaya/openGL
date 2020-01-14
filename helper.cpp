#include "helper.h"

void initShaders() {

    idProgramShader = glCreateProgram();

    idVertexShader = initVertexShader("shader.vert");
    idFragmentShader = initFragmentShader("shader.frag");

    glAttachShader(idProgramShader, idVertexShader);
    glAttachShader(idProgramShader, idFragmentShader);

    glLinkProgram(idProgramShader);

}

GLuint initVertexShader(const string &filename) {
    string shaderSource;

    if (!readDataFromFile(filename, shaderSource)) {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar *shader = (const GLchar *) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    return vs;
}

GLuint initFragmentShader(const string &filename) {
    string shaderSource;

    if (!readDataFromFile(filename, shaderSource)) {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar *shader = (const GLchar *) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    return fs;
}

bool readDataFromFile(const string &fileName, string &data) {
    fstream myfile;

    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open()) {
        string curLine;

        while (getline(myfile, curLine)) {
            data += curLine;
            if (!myfile.eof())
                data += "\n";
        }

        myfile.close();
    } else
        return false;

    return true;
}

void initTexture(char *filegray ,char *filergb, int *w, int *h, GLuint idProgramShader) {
    int width, height;

    unsigned char *raw_image = NULL;
    unsigned char *rgb_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinforgb;
    struct jpeg_decompress_struct cinfogray;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointerr[1];
    JSAMPROW row_pointerg[1];

    FILE *infilegray = fopen(filegray, "rb");
    FILE *infilergb = fopen(filergb, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infilegray) {
        printf("Error opening gray jpeg file %s\n!", filegray);
        return;
    }
    if (!infilergb) {
        printf("Error opening rgb jpeg file %s\n!", filergb);
        return;
    }
    printf("Texture rgb filename = %s\n", filergb);
    printf("Texture gray filename = %s\n", filegray);

    /* here we set up the standard libjpeg error handler */
    cinfogray.err = jpeg_std_error(&jerr);
    cinforgb.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfogray);
    jpeg_create_decompress(&cinforgb);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinforgb, infilergb);
    jpeg_stdio_src(&cinfogray, infilegray);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinforgb, TRUE);
    jpeg_read_header(&cinfogray, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinforgb);
    jpeg_start_decompress(&cinfogray);

    printf("part1\n" );
    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfogray.output_width * cinfogray.output_height * cinfogray.num_components);
    rgb_image = (unsigned char *) malloc(cinfogray.output_width * cinfogray.output_height * cinfogray.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointerr[0] = (unsigned char *) malloc(cinforgb.output_width * cinforgb.num_components);
    row_pointerg[0] = (unsigned char *) malloc(cinfogray.output_width * cinfogray.num_components);
    /* read one scan line at a time */

      printf("part2\n" );
    while (cinfogray.output_scanline < cinfogray.image_height) {
        jpeg_read_scanlines(&cinfogray, row_pointerg, 1);
        for (i = 0; i < cinfogray.image_width * cinfogray.num_components; i++)
            raw_image[location++] = row_pointerg[0][i];
    }
    location = 0;
  printf("part3\n" );
    while (cinforgb.output_scanline < cinforgb.image_height) {
        jpeg_read_scanlines(&cinforgb, row_pointerr, 1);
        for (i = 0; i < cinforgb.image_width * cinforgb.num_components; i++)
            rgb_image[location++] += row_pointerr[0][i];
    }

    height = cinfogray.image_height;
    width = cinfogray.image_width;

    GLint texLoc;
    texLoc = glGetUniformLocation(idProgramShader,"grayTexture");
    glUniform1i(texLoc,0);
    texLoc = glGetUniformLocation(idProgramShader,"rgbTexture");
    glUniform1i(texLoc,1);

    glGenTextures(1 , &idJpegTexture);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, idJpegTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);

    glGenerateMipmap(GL_TEXTURE_2D);

    glGenTextures(1, &idTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, idTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_image);

    *w = width;
    *h = height;
    printf("part5\n" );
    glGenerateMipmap(GL_TEXTURE_2D);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfogray);
    jpeg_destroy_decompress(&cinfogray);
    jpeg_finish_decompress(&cinforgb);
    jpeg_destroy_decompress(&cinforgb);
    free(row_pointerr[0]);
    free(raw_image);
    fclose(infilergb);
    fclose(infilegray);

}


void initSphere(char *filegray ,char *filergb, int *w, int *h, GLuint idProgramShader) {
    int width, height;

    unsigned char *raw_image = NULL;
    unsigned char *rgb_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinforgb;
    struct jpeg_decompress_struct cinfogray;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointerr[1];
    JSAMPROW row_pointerg[1];

    FILE *infilegray = fopen(filegray, "rb");
    FILE *infilergb = fopen(filergb, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infilegray) {
        printf("Error opening gray jpeg file %s\n!", filegray);
        return;
    }
    if (!infilergb) {
        printf("Error opening rgb jpeg file %s\n!", filergb);
        return;
    }
    printf("Texture rgb filename = %s\n", filergb);
    printf("Texture gray filename = %s\n", filegray);

    /* here we set up the standard libjpeg error handler */
    cinfogray.err = jpeg_std_error(&jerr);
    cinforgb.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfogray);
    jpeg_create_decompress(&cinforgb);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinforgb, infilergb);
    jpeg_stdio_src(&cinfogray, infilegray);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinforgb, TRUE);
    jpeg_read_header(&cinfogray, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinforgb);
    jpeg_start_decompress(&cinfogray);

    printf("part1\n" );
    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfogray.output_width * cinfogray.output_height * cinfogray.num_components);
    rgb_image = (unsigned char *) malloc(cinfogray.output_width * cinfogray.output_height * cinfogray.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointerr[0] = (unsigned char *) malloc(cinforgb.output_width * cinforgb.num_components);
    row_pointerg[0] = (unsigned char *) malloc(cinfogray.output_width * cinfogray.num_components);
    /* read one scan line at a time */

      printf("part2\n" );
    while (cinfogray.output_scanline < cinfogray.image_height) {
        jpeg_read_scanlines(&cinfogray, row_pointerg, 1);
        for (i = 0; i < cinfogray.image_width * cinfogray.num_components; i++)
            raw_image[location++] = row_pointerg[0][i];
    }
    location = 0;
  printf("part3\n" );
    while (cinforgb.output_scanline < cinforgb.image_height) {
        jpeg_read_scanlines(&cinforgb, row_pointerr, 1);
        for (i = 0; i < cinforgb.image_width * cinforgb.num_components; i++)
            rgb_image[location++] += row_pointerr[0][i];
    }

    height = cinfogray.image_height;
    width = cinfogray.image_width;

    GLint texLoc;
    texLoc = glGetUniformLocation(idProgramShader,"grayTexture");
    glUniform1i(texLoc,0);
    texLoc = glGetUniformLocation(idProgramShader,"rgbTexture");
    glUniform1i(texLoc,1);

    glGenTextures(1 , &idJpegTexture);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, idJpegTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);

    glGenerateMipmap(GL_TEXTURE_2D);

    glGenTextures(1, &idTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, idTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_image);

    *w = width;
    *h = height;
    printf("part5\n" );
    glGenerateMipmap(GL_TEXTURE_2D);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfogray);
    jpeg_destroy_decompress(&cinfogray);
    jpeg_finish_decompress(&cinforgb);
    jpeg_destroy_decompress(&cinforgb);
    free(row_pointerr[0]);
    free(raw_image);
    fclose(infilergb);
    fclose(infilegray);

}

