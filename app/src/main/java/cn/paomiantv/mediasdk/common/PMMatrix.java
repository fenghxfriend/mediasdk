package cn.paomiantv.mediasdk.common;

import java.util.Arrays;

/**
 * Created by ASUS on 2017/10/21.
 */

public class PMMatrix {
    private float[][] matrix;
    private int m;
    private int n;

    public int getM() {
        return m;
    }

    public int getN() {
        return n;
    }

    public PMMatrix() {
        this(0, 0);
    }

    public PMMatrix(int m, int n) {
        this.m = m;
        this.n = n;
        this.matrix = new float[m][n];
    }

    public PMMatrix(float[][] martix) {
        this.matrix = martix;
        calcDim();
    }

    private void calcDim() {
        m = matrix.length;
        n = matrix[0].length;
    }

    public void setPMMatrixElement(int i, int j, float value) {

        try {
            if (i > m || j > n)
                throw new Exception("索引越界");
        } catch (Exception e) {
            e.printStackTrace();
        }
        matrix[i][j] = value;
    }

    public boolean loadIdentity() {
        boolean re = false;
        if (m != n) {
            try {
                throw new Exception("非m*m的矩阵，无法初始化为单位矩阵");
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            for (int i = 0; i < m; i++) {
                setPMMatrixElement(i, i, 1.0f);
            }
        }

        return re;
    }

    public float getMartixElement(int i, int j) {
        return matrix[i][j];
    }

    /**
     * 求矩阵M转置
     *
     * @return 转置矩阵
     */
    public PMMatrix tranpose() {
        float[][] temp = new float[n][m];
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                temp[j][i] = matrix[i][j];
            }
        }
        return new PMMatrix(temp);
    }

    /**
     * 矩阵相加
     *
     * @param matrix
     * @return
     */
    public PMMatrix add(PMMatrix matrix) {
        try {
            if (matrix.getM() != m || matrix.getN() != n)
                throw new Exception("矩阵维数不相等，不能相加");
        } catch (Exception e) {
            e.printStackTrace();
        }

        float[][] temp = new float[m][n];
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                temp[i][j] = matrix.getMartixElement(i, j) + this.matrix[i][j];
            }
        }

        return new PMMatrix(temp);
    }

    /**
     * 矩阵相减
     *
     * @param matrix
     * @return
     */
    public PMMatrix reduce(PMMatrix matrix) {
        try {
            if (matrix.getM() != m || matrix.getN() != n)
                throw new Exception("矩阵维数不相等，不能相加");
        } catch (Exception e) {
            e.printStackTrace();
        }

        float[][] temp = new float[m][n];
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                temp[i][j] = this.matrix[i][j] - matrix.getMartixElement(i, j);
            }
        }

        return new PMMatrix(temp);
    }

    /**
     * 矩阵相乘
     *
     * @param matrix
     * @return
     */
    public PMMatrix multiply(PMMatrix matrix) {

        try {
            if (matrix.getM() != n)
                throw new Exception("矩阵维数不符合相乘要求");
        } catch (Exception e) {
            e.printStackTrace();
        }

        float[][] temp = new float[m][matrix.getN()];

        for (int i = 0; i < m; i++) {
            for (int k = 0; k < matrix.getN(); k++) {
                float t = 0;
                for (int j = 0; j < n; j++) {
                    t += this.matrix[i][j] * matrix.getMartixElement(j, k);
                }

                temp[i][k] = t;
            }
        }
        return new PMMatrix(temp);
    }


    /**
     * 格式化输出
     */
    public String toFormatString() {
        StringBuffer buf = new StringBuffer();
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                buf.append(matrix[i][j]).append("/t");
            }
            buf.append("/n");
        }

        return buf.toString();
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + m;
        result = prime * result + Arrays.hashCode(matrix);
        result = prime * result + n;
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        PMMatrix other = (PMMatrix) obj;
        if (m != other.m)
            return false;
        if (!Arrays.equals(matrix, other.matrix))
            return false;
        if (n != other.n)
            return false;
        return true;
    }
}
