#include "component.h"
#include "cimage.h"
#include "bmplib.h"
#include <deque>
#include <iomanip>
#include <iostream>
#include <cmath>
// You shouldn't need other #include's - Ask permission before adding

using namespace std;

// TO DO: Complete this function
CImage::CImage(const char* bmp_filename)
{
    //  Note: readRGBBMP dynamically allocates a 3D array
    //    (i.e. array of pointers to pointers (1 per row/height) where each
    //    point to an array of pointers (1 per col/width) where each
    //    point to an array of 3 unsigned char (uint8_t) pixels [R,G,B values])

    // ================================================
    // TO DO: call readRGBBMP to initialize img_, h_, and w_;
    img_ = readRGBBMP(bmp_filename, h_, w_);


    // Leave this check
    if(img_ == NULL) {
        throw std::logic_error("Could not read input file");
    }

    // Set the background RGB color using the upper-left pixel
    for(int i=0; i<=2; i++) 
    {
        bgColor_[i] = img_[0][0][i];
    }

    // ======== This value should work - do not alter it =======
    // RGB "distance" threshold to continue a BFS from neighboring pixels
    bfsBgrdThresh_ = 60;

    // ================================================
    // TO DO: Initialize the vector of vectors of labels to -1
    for (int h = 0; h < h_; h++) 
    {
        //adds width and makes values -1
        labels_.push_back(vector<int>(w_, -1));
        
    }
    // ================================================
    // TO DO: Initialize any other data members
    count = 0;
//
}

// TO DO: Complete this function
CImage::~CImage()
{
    // Add code here if necessary
   deallocateImage(img_);
}

// Complete - Do not alter
bool CImage::isCloseToBground(uint8_t p1[3], double within) {
    // Computes "RGB" (3D Cartesian distance)
    double dist = sqrt( pow(p1[0]-bgColor_[0],2) +
                        pow(p1[1]-bgColor_[1],2) +
                        pow(p1[2]-bgColor_[2],2) );
    return dist <= within;
}

// TO DO: Complete this function
size_t CImage::findComponents()
{
  for(int h = 0; h < h_; h++)
    {
        for(int w = 0; w < w_; w++)
        {
            //makes sure spot is valid
            while((labels_[h][w] == -1))
            {
                if((!isCloseToBground(img_[h][w], bfsBgrdThresh_)))
                {
                //cout << "here";
                //bfs componnent
                Component comp = bfsComponent(h, w, count);
                //adds to comp
                components_.push_back(comp);
                count++;
                }
                break;
            }
        }
    }
    int ans = count+1;
    return ans;
}

// Complete - Do not alter
void CImage::printComponents() const
{
    cout << "Height and width of image: " << h_ << "," << w_ << endl;
    cout << setw(4) << "Ord" << setw(4) << "Lbl" << setw(6) << "ULRow" << setw(6) << "ULCol" << setw(4) << "Ht." << setw(4) << "Wi." << endl;
    for(size_t i = 0; i < components_.size(); i++) {
        const Component& c = components_[i];
        cout << setw(4) << i << setw(4) << c.label << setw(6) << c.ulNew.row << setw(6) << c.ulNew.col
             << setw(4) << c.height << setw(4) << c.width << endl;
    }

}


// TODO: Complete this function
int CImage::getComponentIndex(int mylabel) const
{
   bool found = false;
    for (size_t i = 0; i < components_.size(); i++) 
    {
        if (components_[i].label == mylabel) 
        {
            found = true;
            return i;
        }
    }

    if(!found)
    {
        return -1;
    }
}


// Nearly complete - TO DO:
//   Add checks to ensure the new location still keeps
//   the entire component in the legal image boundaries
void CImage::translate(int mylabel, int nr, int nc)
{
    // Get the index of specified component
    int cid = getComponentIndex(mylabel);
    if(cid < 0) {
        return;
    }
    int h = components_[cid].height;
    int w = components_[cid].width;

    // ==========================================================
    // ADD CODE TO CHECK IF THE COMPONENT WILL STILL BE IN BOUNDS
    // IF NOT:  JUST RETURN.
    bool valid = false;
    if((nr >= 0) && (nr + h <= h_) && (nc >= 0) && (nc + w <= w_))
    {
      valid = true;
    }

    if(!valid)
    {
        return;
    }
    // ==========================================================

    // If we reach here we assume the component will still be in bounds
    // so we update its location.
    Location nl(nr, nc);
    components_[cid].ulNew = nl;
}

// TO DO: Complete this function
void CImage::forward(int mylabel, int delta)
{
    int cid = getComponentIndex(mylabel);
    if(cid < 0 || delta <= 0) {
        return;
    }
    // Add your code here
    //makes sure the change value is right
     int newdelta = min(delta, count - cid - 1);

     for (int i = 0; i < newdelta; i++) 
     {
        //swap components forward
        swap(components_[cid + i], components_[cid + i + 1]);
     }
}

// TO DO: Complete this function
void CImage::backward(int mylabel, int delta)
{
    int cid = getComponentIndex(mylabel);
    if(cid < 0 || delta <= 0) {
        return;
    }
    // Add your code here
    //makes sure change value is right
    int newdelta = min(delta, cid);

    for (int i = 0; i < newdelta; i++) 
     {
        //swap components backwards
        swap(components_[cid - i], components_[cid - i - 1]);
     }
}

// TODO: complete this function
void CImage::save(const char* filename)
{
    // Create another image filled in with the background color
    uint8_t*** out = newImage(bgColor_);

    // Add your code here
    for (size_t x = 0; x < components_.size(); x++) 
    {
        Component comp = components_[x];
        //make sure all the bounds are right
        if (comp.ulOrig.row < 0 || comp.ulOrig.col < 0 
        || comp.ulNew.row < 0 || comp.ulNew.col < 0 
        || comp.ulOrig.row + comp.height > h_ 
        || comp.ulOrig.col + comp.width > w_ 
        || comp.ulNew.row + comp.height > h_ 
        || comp.ulNew.col + comp.width > w_) 
        {
            continue; 
        }

        for (int i = 0; i < comp.height; ++i) 
        {
            for (int j = 0; j < comp.width; ++j) 
            {
                //updates values
                int lastrow = comp.ulOrig.row + i;
                int lastcol = comp.ulOrig.col + j;
                int newrow = comp.ulNew.row + i;
                int newcol = comp.ulNew.col + j;

                if (lastrow >= 0 && lastrow < h_ && lastcol >= 0 && lastcol < w_ &&
                    newrow >= 0 && newrow < h_ && newcol >= 0 && newcol < w_ &&
                    labels_[lastrow][lastcol] == comp.label) {
                    // assigns to out
                    out[newrow][newcol][0] = img_[lastrow][lastcol][0];
                    out[newrow][newcol][1] = img_[lastrow][lastcol][1];
                    out[newrow][newcol][2] = img_[lastrow][lastcol][2];
                }
            }
        }
    }

    writeRGBBMP(filename, out, h_, w_);
    // Add any other code you need after this
    deallocateImage(out);
}

// Complete - Do not alter - Creates a blank image with the background color
uint8_t*** CImage::newImage(uint8_t bground[3]) const
{
    uint8_t*** img = new uint8_t**[h_];
    for(int r=0; r < h_; r++) {
        img[r] = new uint8_t*[w_];
        for(int c = 0; c < w_; c++) {
            img[r][c] = new uint8_t[3];
            img[r][c][0] = bground[0];
            img[r][c][1] = bground[1];
            img[r][c][2] = bground[2];
        }
    }
    return img;
}

// To be completed
void CImage::deallocateImage(uint8_t*** img) const
{
    for (int r = 0; r < h_; r++)
    {
        for (int c = 0; c < w_; c++)
        {
            delete[] img[r][c];
        }
        delete[] img[r];
    }
    delete[] img;
}

// TODO: Complete the following function or delete this if
//       you do not wish to use it.
Component CImage::bfsComponent(int pr, int pc, int mylabel)
{
    // Arrays to help produce neighbors easily in a loop
    // by encoding the **change** to the current location.
    // Goes in order N, NW, W, SW, S, SE, E, NE
    int neighbor_row[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int neighbor_col[8] = {0, -1, -1, -1, 0, 1, 1, 1};
    Location l(pr, pc);
    labels_[pr][pc] = mylabel;
    deque<Location> myque;
    myque.push_back(l);
    int rowmax = pr; 
    int colmax = pc;
    int rowmin = pr; 
    int colmin = pc;

    while (!(myque.empty()))
    {
        //get location
        Location loc = myque.front();
        //add to front of que
        myque.pop_front();

        //getting maximums and minumums
        rowmax = max(rowmax, loc.row);
        rowmin = min(rowmin, loc.row);
        colmax = max(colmax, loc.col);
        colmin = min(colmin, loc.col);

        for(int i = 0; i<8; i++)
        {
            Location next; 
            bool valid = false;
            // next column index
            next.col = neighbor_col[i] + loc.col;
            //next row index
            next.row = neighbor_row[i] + loc.row;
            if((next.row >= 0) && (next.row < h_) && (next.col >= 0) && (next.col < w_))
            {
                valid = true; 
            }

            while(valid)
            {
                //cout <<"here";
                if ((labels_[next.row][next.col] == -1) && (!isCloseToBground(img_[next.row][next.col], bfsBgrdThresh_)))
                    {
                        //cout <<"here";
                        labels_[next.row][next.col] = mylabel;
                        myque.push_back(next);
                    }
                break;
            }
        }

    }

    l.col = colmin;
    Component ans(l, rowmax - rowmin + 1, colmax - colmin + 1, mylabel);
    return ans;

}

// Complete - Debugging function to save a new image
void CImage::labelToRGB(const char* filename)
{
    //multiple ways to do this -- this is one way
    vector<uint8_t[3]> colors(components_.size());
    for(unsigned int i=0; i<components_.size(); i++) {
        colors[i][0] = rand() % 256;
        colors[i][1] = rand() % 256;
        colors[i][2] = rand() % 256;
    }

    for(int i=0; i<h_; i++) {
        for(int j=0; j<w_; j++) {
            int mylabel = labels_[i][j];
            if(mylabel >= 0) {
                img_[i][j][0] =  colors[mylabel][0];
                img_[i][j][1] =  colors[mylabel][1];
                img_[i][j][2] =  colors[mylabel][2];
            } else {
                img_[i][j][0] = 0;
                img_[i][j][1] = 0;
                img_[i][j][2] = 0;
            }
        }
    }
    writeRGBBMP(filename, img_, h_, w_);
}

// Complete - Do not alter
const Component& CImage::getComponent(size_t i) const
{
    if(i >= components_.size()) {
        throw std::out_of_range("Index to get Component is out of range");
    }
    return components_[i];
}

// Complete - Do not alter
size_t CImage::numComponents() const
{
    return components_.size();
}

// Complete - Do not alter
void CImage::drawBoundingBoxesAndSave(const char* filename)
{
    for(size_t i=0; i < components_.size(); i++){
        Location ul = components_[i].ulOrig;
        int h = components_[i].height;
        int w = components_[i].width;
        for(int i = ul.row; i < ul.row + h; i++){
            for(int k = 0; k < 3; k++){
                img_[i][ul.col][k] = 255-bgColor_[k];
                img_[i][ul.col+w-1][k] = 255-bgColor_[k];

            }
            // cout << "bb " << i << " " << ul.col << " and " << i << " " << ul.col+w-1 << endl; 
        }
        for(int j = ul.col; j < ul.col + w ; j++){
            for(int k = 0; k < 3; k++){
                img_[ul.row][j][k] = 255-bgColor_[k];
                img_[ul.row+h-1][j][k] = 255-bgColor_[k];

            }
            // cout << "bb2 " << ul.row << " " << j << " and " << ul.row+h-1 << " " << j << endl; 
        }
    }
    writeRGBBMP(filename, img_, h_, w_);
}

// Add other (helper) function definitions here


