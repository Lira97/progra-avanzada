int kCenterX = kCol / 2;
int kCenterY = kRow / 2;

for(int n=0; n < mRow; ++n)
{              // rows
    for(int m=0; m < mCol; ++m)
    {          // columns

        for(int m=0; m < kRow; ++m)
        {     // kernel rows
            int mm = kRow - 1 - m;      // row index of flipped kernel
            for(int n=0; n < kCol; ++n)
            { // kernel columns
                int nn = kCol - 1 - n;  // column index of flipped kernel

                // index of input signal, used for checking boundari
                int ii = n + (m - kCenterY);
                int jj = m + (n - kCenterX);

                if(ii < 0)
                    ii=ii+1;
                if(jj < 0)
                    jj=jj+1;
                if(ii >= mRow)
                    ii=ii-1;
                if(jj >= mCol)
                    jj=jj-1;
                if( ii >= 0 && ii < mRow && jj >= 0 && jj < mCol )
                  newImage[d][n][m] += filter[h-n][w-m]*image[d][h][w];
                    o[n][m] += input[ii][jj] * kern[mm][n];

            }
        }
    }
}
int ms = MS;
int ref = floor(5/2);

int n, m, ii, jj;

//#pragma omp parallel for private(n,m,ii,jj) shared(input, output)

for ( n = 0 ; n < image->height ; n ++)
{
    for ( m = 0 ; m < image->width ; m++)
    {

        int blue = 0;
        int green = 0;
        int red = 0;

        int contador = 0;

        for ( ii = -ref ; ii <= ref ; ii++)
        {
            for ( jj = - ref ; jj <=ref ; jj++){

                if ( n + ii < 0 && m + jj < 0 && m + jj >= rows && n + ii >= cols ){

                    //Casilla de la matriz aux fuera de la imagen
                }
                else
                {
                    contador++;

                    blue += image->pixels[m+jj][n+ii].data[0];
                    green += image->pixels[m+jj][n+ii].data[1];
                    red += image->pixels[m+jj][n+ii].data[2];

                }

            }
        }

        image->pixels[m][n].data[0] = (blue/contador);
        image->pixels[m][n].data[1] = (green/contador);
        image->pixels[m][n].data[2] = (red/contador);

    }
}
int kern[3][3] = {{1 ,0,-1}, {0,0,0},{-1 ,0 ,1}};
int kern[3][3] = {{0 ,-1,0}, {-1,5,-1},{0 ,-1,0}};
for (int color=0; color<3; color++) {
    for (n=0 ; n<image->height ; n++) {
        for (m=0 ; m<image->width ; m++) {
            for (h=n ; h<n+3 ; h++) {
                for (w=m ; w<m+3 ; w++) {
                  image->pixels[n][m].data[color] += kern[h-n][w-m]*image->pixels[n][m].data[color]

                }
            }
        }
    }
}

int kw = 3;
int kh = 3;
int kernel[] = {0, 0, 0,
                  0, 1, 0,
                  0, 0, 0};

int ref = (kw - 1) / 2;
int ref = (kh - 1) / 2;

for (int n = 0; n < image->height; n++) {

    for (int j = 0; j < image->width; j++) {
        int pixelR = 0;
        int pixelG = 0;
        int pixelB = 0;

        // Appli convolution to each channel.
        for (int m = 0; m < kh; m++)
        {

            for (int n = 0; n < kw; n++)
            {

                    pixelR +=  image->pixels[n + m - ref][j + n - ref].data[0] * k;
                    pixelG +=  image->pixels[n + m - ref][j + n - ref].data[1] * k;
                    pixelB +=  image->pixels[n + m - ref][j + n - ref].data[2] * k;
            }
        }

        mage->pixels[i][j].data[0] = pixelR;
        image->pixels[i][j].data[1] = pixelG;
        image->pixels[i][j].data[2] = pixelB;

    }
}





int kCenterX = kCol / 2;
int kCenterY = kRow / 2;

for(int i=0; i < mRow; ++i) {              // rows
    for(int j=0; j < mCol; ++j) {          // columns

        for(int m=0; m < kRow; ++m) {     // kernel rows
            int mm = kRow - 1 - m;      // row index of flipped kernel
            for(int n=0; n < kCol; ++n) { // kernel columns
                int nn = kCol - 1 - n;  // column index of flipped kernel

                // index of input signal, used for checking boundary
                int ii = i + (m - kCenterY);
                int jj = j + (n - kCenterX);

                if(ii < 0)
                    ii=ii+1;
                if(jj < 0)
                    jj=jj+1;
                if(ii >= mRow)
                    ii=ii-1;
                if(jj >= mCol)
                    jj=jj-1;
                if( ii >= 0 && ii < mRow && jj >= 0 && jj < mCol )
                    o[i][j] += input[ii][jj] * kern[mm][n];

            }
        }
    }
}

    int kw = 3;
    int kh = 3;
    qreal kernel[] = {0, 0, 0,
                      0, 1, 0,
                      0, 0, 0};

    int ref = (kw - 1) / 2;

    int ref = floor((3 - 1) / 2);

    for (int y = 0; y < inImage.height(); y++) {
        QRgb *outLine = (QRgb *) outImage.scanLine(y);

        for (int x = 0; x < inImage.width(); x++) {
            qreal pixelR = 0;
            qreal pixelG = 0;
            qreal pixelB = 0;

            // Apply convolution to each channel.
            for (int j = 0; j < kh; j++) {
                if (y + j < ref
                    || y + j - ref >= inImage.height())
                    continue;

                const QRgb *inLine = (QRgb *) inImage.constScanLine(y + j - ref);

                for (int i = 0; i < kw; i++) {
                    if (x + i < ref
                        || x + i - ref >= inImage.width())
                        continue;

                    qreal k = kernel[i + j * kw];
                    QRgb pixel = inLine[x + i - ref];

                    pixelR += k * qRed(pixel);
                    pixelG += k * qGreen(pixel);
                    pixelB += k * qBlue(pixel);
                }
            }

            quint8 r = qBound(0., pixelR, 255.);
            quint8 g = qBound(0., pixelG, 255.);
            quint8 b = qBound(0., pixelB, 255.);
            outLine[x] = qRgb(r, g, b);
        }

    }






for ( int row = 0; row < image->height; row++ ) {
  for ( int col = 0; col < image->width; col++ ) {
    int blue = 0;
    int green = 0;
    int red = 0;
    int sum = 0;
  for ( int i = -1; i <= 1; i++ ) {
    for ( int j = -1; j <= 1; j++ ) {
      blue += image->pixels[row+i][col+j].data[0]* kern[1+i][1+j];
      green += image->pixels[row+i][col+j].data[1]* kern[1+i][1+j];
      red += image->pixels[row+i][col+j].data[2]* kern[1+i][1+j];
      sum += kernel[1+i][1+j];
    }
  }
  image->pixels[row][col].data[0]=(blue/sum);
  image->pixels[row][col].data[1]=(green/sum);
  image->pixels[row][col].data[2]=(red/sum);

}







void Filter(ppm_t * image){
        int kSize = 3;
//        int kern[3][3] = {{-2 ,-1,0}, {-1,1,1},{0 ,1,2}};//Emboss
//        int kern[3][3] = {{0 ,1,0}, {1,-4,1},{0 ,1,0}};//Edge detect
//        int kern[3][3] = {{0 ,0,0}, {-1,1,0},{0 ,0,0}};//Edge Enhance


        //int kern[3][3] = {{1,1,1}, {1,1,1}, {1,1,1}};//BOX Blur
//        int kern[3][3] = {{1,2,1}, {2,4,2}, {1,2,1}};//Blur Gausian 1
        //int kern[5][5] = {{1,4,6,4,1}, {4,16,24,16,4}, {6,24,36,24,6}, {4,16,24,16,4}, {1,4,6,4,1}};//Blur Gausian 2
        //kSize = 5;
//        int kern[3][3] = {{0, -1,0}, {-1, 5, -1}, {0,-1,0}};//Sharpen Wikipedia

int kCenterX = floor(matrix->height / 2);
int kCenterY = floor(matrix->width / 2);
        for (int i=0; i< image->height; ++i)
        {
            for (int j=0; j< image->width; ++j)
            {
              int blue = 0;
              int green = 0;
              int red = 0;
              int sum = 0;

              for(int m= kCenterY; m < kSize; ++m){     // kernel rows
                  int mm = kSize - 1 - m;      // row index of flipped kernel
                  for(int n= kCenterX; n < kSize; ++n)
                  { // kernel columns
                      int nn = kSize - 1 - n;  // column index of flipped kernel

                      // index of input signal, used for checking boundary
                      int ii = i + (kCenterY-mm);
                      int jj = j + (kCenterX-nn);
                      if(ii < 0)
                          ii=ii+1;
//                          ii=ii*-1;
                      if(jj < 0)
                          jj=jj+1;
//                          jj=jj*-1;
                      if(ii >= image->height)
                          ii=ii-1;
//                          ii=ii*-1;
                      if(jj >= image->width)
                          jj=jj-1;
//                          jj=jj*-1;
                      if( ii >= 0 && ii < image->height && jj >= 0 && jj < image->width ){
                        sum = (sum + matrix->values[mm][nn]);
                        blue += image->pixels[ii][jj].data[0] * matrix->values[mm][nn];
                        green += image->pixels[ii][jj].data[1] * matrix->values[mm][nn];
                        red += image->pixels[ii][jj].data[2] * matrix->values[mm][nn];

                      }
                  }

              }
              if(sum == 0)
              {
              image->pixels[i][j].data[0]=blue;
              image->pixels[i][j].data[1]=green;
              image->pixels[i][j].data[2]=red;
              }
              else
              {

                image->pixels[i][j].data[0]=blue/sum;
                image->pixels[i][j].data[1]=green/sum;
                image->pixels[i][j].data[2]=red/sum;
              }
            }
        }





        for (ii = -kCenterX ; ii <= kCenterX; ii++){
                for (jj = -kCenterY; jj <= kCenterY;  jj++){

                    if (i + ii > 0 && j + jj > 0 && j + jj < rows && i + ii < cols){

                        int x = ii + ref;
                        int y = jj + ref;

                        sum += matrix->values[x][y];
                        blue += image->pixels[i+ii][j+jj].data[0] * matrix->values[x][y];
                        green += image->pixels[i+ii][j+jj].data[1] * matrix->values[x][y];
                        red += image->pixels[i+ii][j+jj].data[2] * matrix->values[x][y];


                        //printf("xsi: %d, y: %d, m: %f\n", x,y,filter->m[x][y]);


                    }
                    else {


                        //printf("xno: %d, y: %d, m: %f\n", x,y,filter->m[x][y]);
                    }

                }
            }

}



for ( row = 0; row < image->height; row++ ) {
  for ( col = 0; col < image->width; col++ ) {
    int blue = 0;
        int green = 0;
        int red = 0;
        int sum = 0;
  for ( i = -1; i <= 1; i++ ) {
    for ( j = -1; j <= 1; j++ ) {
      blue+= image->pixels[row+i][col+j].data[0] * matrix->values[1+i][1+j];
      green += image->pixels[row+i][col+j].data[1] * matrix->values[1+i][1+j];
      red += image->pixels[row+i][col+j].data[2] * matrix->values[1+i][1+j];
      weightsum += kernel[1+i][1+j];
    }
  }
  image->pixels[row][col].data[0]=blue/sum;
  image->pixels[row][col].data[1]=green/sum;
  image->pixels[row][col].data[2]=red/sum;
}
