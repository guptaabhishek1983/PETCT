namespace MPR_UI
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.button1 = new System.Windows.Forms.Button();
            this.axialPanel = new System.Windows.Forms.Panel();
            this.sagittalPanel = new System.Windows.Forms.Panel();
            this.coronalPanel = new System.Windows.Forms.Panel();
            this.tableLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.BackColor = System.Drawing.SystemColors.ActiveBorder;
            this.tableLayoutPanel1.CellBorderStyle = System.Windows.Forms.TableLayoutPanelCellBorderStyle.Outset;
            this.tableLayoutPanel1.ColumnCount = 3;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel1.Controls.Add(this.button1, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.axialPanel, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.sagittalPanel, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.coronalPanel, 2, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 30F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(723, 510);
            this.tableLayoutPanel1.TabIndex = 0;
            this.tableLayoutPanel1.Paint += new System.Windows.Forms.PaintEventHandler(this.tableLayoutPanel1_Paint);
            // 
            // button1
            // 
            this.button1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.button1.Location = new System.Drawing.Point(5, 481);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(232, 24);
            this.button1.TabIndex = 0;
            this.button1.Text = "Load CT";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // axialPanel
            // 
            this.axialPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.axialPanel.Location = new System.Drawing.Point(5, 5);
            this.axialPanel.Name = "axialPanel";
            this.axialPanel.Size = new System.Drawing.Size(232, 468);
            this.axialPanel.TabIndex = 1;
            // 
            // sagittalPanel
            // 
            this.sagittalPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.sagittalPanel.Location = new System.Drawing.Point(245, 5);
            this.sagittalPanel.Name = "sagittalPanel";
            this.sagittalPanel.Size = new System.Drawing.Size(232, 468);
            this.sagittalPanel.TabIndex = 2;
            // 
            // coronalPanel
            // 
            this.coronalPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.coronalPanel.Location = new System.Drawing.Point(485, 5);
            this.coronalPanel.Name = "coronalPanel";
            this.coronalPanel.Size = new System.Drawing.Size(233, 468);
            this.coronalPanel.TabIndex = 3;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(723, 510);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Name = "MainForm";
            this.Text = "MPR Viewer";
            this.WindowState = System.Windows.Forms.FormWindowState.Maximized;
            this.Resize += new System.EventHandler(this.MainForm_Resize);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Panel axialPanel;
        private System.Windows.Forms.Panel sagittalPanel;
        private System.Windows.Forms.Panel coronalPanel;

    }
}

